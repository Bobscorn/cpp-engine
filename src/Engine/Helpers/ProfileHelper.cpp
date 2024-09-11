#include "ProfileHelper.h"

#include "DebugHelper.h"

#include <iostream>
#include <fstream>
#include <cstdio>
#include <functional>
#include <algorithm>

#ifdef EC_PROFILE
constexpr float OtherThreshold = 0.001f;

bool TimedEvent::operator==(const TimedEvent& other) const
{
	return Name == other.Name
		&& PercentOfFrame == other.PercentOfFrame
		&& PercentOfParent == other.PercentOfParent
		&& Time == other.Time
		&& StartTime == other.StartTime
		&& Parent == other.Parent
		&& EndTime == other.EndTime
		&& ChildEvents == other.ChildEvents;
}

void TimedEvent::AggregateChildren()
{
	if (!Aggregate)
		return;

	bool foundSome = true;
	while (foundSome)
	{
		foundSome = false;
		for (int i = ChildEvents.size(); i-- > 0; )
		{
			for (int j = ChildEvents.size(); j-- > 0; )
			{
				if (i == j)
					continue;
				if (ChildEvents[i].Name == ChildEvents[j].Name &&
					ChildEvents[i].Aggregate && ChildEvents[j].Aggregate)
				{
					foundSome = true;
					ChildEvents[i].AggregateOntoThis(ChildEvents[j], *this);
					goto leaveloop;
				}
			}
		}
	leaveloop:;
	}
}

void TimedEvent::AggregateOntoThis(TimedEvent& other, TimedEvent& parent)
{
	if (other.Name != Name)
		return;

	int siblingIndex = -1;
	for (int i = 0; i < parent.ChildEvents.size(); ++i)
		if (parent.ChildEvents[i] == other)
			siblingIndex = i;

	if (siblingIndex == -1)
		return;

	Time += other.Time;
	Aggregations += other.Aggregations + 1;

	int start = (int)ChildEvents.size();
	ChildEvents.insert(ChildEvents.end(), other.ChildEvents.begin(), other.ChildEvents.end());

	AggregateChildren();

	parent.ChildEvents.erase(parent.ChildEvents.begin() + siblingIndex);	
}

ProfileMcGee::ProfileMcGee(ProfileOptions options) : m_Options(options)
{
	m_TimerBoi.Reset();
	m_TimerBoi.Stop();
}

#ifdef _DEBUG
void ProfileMcGee::Push(std::string name, const char *func, size_t line)
#else


void ProfileMcGee::Push(std::string name)
#endif // _DEBUG
{
	if (!m_Running)
		return;

	if (!m_CurrentChild)
		return;
	m_TimerBoi.Tick();
#ifdef _DEBUG
	m_CurrentChild = &m_CurrentChild->ChildEvents.emplace_back(TimedEvent{ name, m_TimerBoi.TotalTime(), m_CurrentChild, func, line });
#else
	m_CurrentChild = &m_CurrentChild->ChildEvents.emplace_back(TimedEvent{ name, m_TimerBoi.TotalTime(), m_CurrentChild });
#endif // _DEBUG
}
#ifdef _DEBUG
void ProfileMcGee::PushAggregate(std::string name, const char* func, size_t line)
#else
void ProfileMcGee::PushAggregate(std::string name)
#endif
{
	if (!m_Running)
		return;

	if (!m_CurrentChild)
		return;
	m_TimerBoi.Tick();
#ifdef _DEBUG
	m_CurrentChild = &m_CurrentChild->ChildEvents.emplace_back(TimedEvent{ name, m_TimerBoi.TotalTime(), m_CurrentChild, func, line, true });
#else
	m_CurrentChild = &m_CurrentChild->ChildEvents.emplace_back(TimedEvent{ name, m_TimerBoi.TotalTime(), m_CurrentChild, true });
#endif // _DEBUG
}

#ifdef _DEBUG
void ProfileMcGee::Pop(const char *func, size_t line)
#else
void ProfileMcGee::Pop()
#endif
{
	if (!m_Running)
		return;

	if (!m_CurrentChild)
		return;
	if (m_CurrentChild == m_CurrentFrame)
		return;

#ifdef _DEBUG
	if (strcmp(m_CurrentChild->Signature, func))
	{
		DWARNING("The pop for: " + m_CurrentChild->Signature + '(' + std::to_string(m_CurrentChild->Line) + ')' + " was missed, the pop attempted is at: " + func + '(' + std::to_string(line) + ')');
		return;
	}
#endif

	m_TimerBoi.Tick();
	m_CurrentChild->EndTime = m_TimerBoi.TotalTime();
	m_CurrentChild->Time = m_CurrentChild->EndTime - m_CurrentChild->StartTime;

	// Add extra time as 'Other'
	if (m_CurrentChild->ChildEvents.size())
	{
		double last_time = m_CurrentChild->ChildEvents.back().EndTime;
		if (m_CurrentChild->EndTime > last_time + OtherThreshold)
		{
			auto &other = m_CurrentChild->ChildEvents.emplace_back(TimedEvent{ "Other", last_time, m_CurrentChild });
			other.EndTime = m_CurrentChild->EndTime;
			other.Time = other.EndTime - other.StartTime;
		}
	}

	// Aggregate
	if (m_CurrentChild->Parent && m_CurrentChild->Aggregate)
	{
		for (int i = 0; i < m_CurrentChild->Parent->ChildEvents.size(); ++i)
		{
			auto& sibling = m_CurrentChild->Parent->ChildEvents[i];

			if (sibling != *m_CurrentChild &&
				sibling.Name == m_CurrentChild->Name &&
				sibling.Aggregate)
			{
				auto parent = m_CurrentChild->Parent;

				m_CurrentChild->AggregateOntoThis(sibling, *parent);

				m_CurrentChild = parent;
				return;
			}
		}
	}

	m_CurrentChild = m_CurrentChild->Parent;
}

void ProfileMcGee::BeginFrame()
{
	if (!m_Running)
		return;

	m_TimerBoi.Start();
	m_TimerBoi.Tick();
	m_WorkingFrame = TimedEvent{ "Frame " + std::to_string(m_Stats.Frames.size()), m_TimerBoi.TotalTime() };

	m_CurrentFrame = &m_WorkingFrame;
	m_CurrentChild = m_CurrentFrame;
}

TimedEvent ProfileMcGee::EndFrame()
{
	if (!m_Running)
		return TimedEvent{};

	if (!m_CurrentFrame)
		return TimedEvent();

	m_TimerBoi.Tick();
	m_CurrentFrame->EndTime = m_TimerBoi.TotalTime();

	// Close remaining events
	while (m_CurrentChild != m_CurrentFrame && m_CurrentChild)
	{
#ifdef _DEBUG
		if (m_CurrentChild->Signature)
			DWARNING(m_CurrentChild->Name + " not closed in profiler pushed at: " + m_CurrentChild->Signature + '(' + std::to_string(m_CurrentChild->Line) + ')');
		else
			DWARNING(m_CurrentChild->Name + " not closed in profiler");
		
#endif // _DEBUG
		m_TimerBoi.Tick();
		m_CurrentChild->EndTime = m_TimerBoi.TotalTime();
		m_CurrentChild->Time = m_CurrentChild->EndTime - m_CurrentChild->StartTime;

		// Add extra time as 'Other'
		if (m_CurrentChild->ChildEvents.size())
		{
			double last_time = m_CurrentChild->ChildEvents.back().EndTime;
			if (m_CurrentChild->EndTime > last_time + OtherThreshold)
			{
				auto &other = m_CurrentChild->ChildEvents.emplace_back(TimedEvent{ "Other", last_time, m_CurrentChild });
				other.EndTime = m_CurrentChild->EndTime;
				other.Time = other.EndTime - other.StartTime;
			}
		}

		m_CurrentChild = m_CurrentChild->Parent;
	}

	// Insert extra time as another event
	if (m_CurrentFrame->ChildEvents.size())
		m_CurrentFrame->ChildEvents.emplace_back(TimedEvent{ "Other", m_CurrentFrame->ChildEvents.back().EndTime, m_CurrentFrame });
	else
		m_CurrentFrame->ChildEvents.emplace_back(TimedEvent{ "Other", m_CurrentFrame->StartTime, m_CurrentFrame });
	auto &thingo = m_CurrentFrame->ChildEvents.back();
	thingo.EndTime = m_CurrentFrame->EndTime;
	thingo.Time = thingo.EndTime - thingo.StartTime;	

	m_TimerBoi.Stop();
	m_CurrentFrame->Time = m_CurrentFrame->EndTime - m_CurrentFrame->StartTime;
	m_Stats.Frames.push_back(m_WorkingFrame);
	TimedEvent tmp = m_WorkingFrame;
	m_WorkingFrame = TimedEvent{};
	m_CurrentFrame = nullptr;
	m_CurrentChild = nullptr;
	return tmp;
}

void ProfileMcGee::Start()
{
	m_Stats.Frames.clear();
	m_Running = true;
}

const BigBoiStats &ProfileMcGee::Finish(bool fillallframes)
{
	if (!m_Running)
		return *(BigBoiStats*)nullptr; // Will throw a access violation if this is *accessed*
	m_Running = false;
	return Query(fillallframes);
}

const BigBoiStats &ProfileMcGee::Query(bool fillallframes)
{
	m_Stats.m_AverageTime = 0.0;
	m_Stats.m_RangeLow = 9999999999.0;
	m_Stats.m_RangeHigh = 0.0;
	if (m_Stats.Frames.size())
	{
		m_Stats.LongestFrame = &m_Stats.Frames.front();
		m_Stats.ShortestFrame = &m_Stats.Frames.front();
	}
	auto &longf = m_Stats.LongestFrame;
	auto &shortf = m_Stats.ShortestFrame;
	double total_time = 0.0;
	for (auto &frame : m_Stats.Frames)
	{
		if (frame.Time > longf->Time)
			longf = &frame;
		if (frame.Time < shortf->Time)
			shortf = &frame;
		total_time += frame.Time;
		m_Stats.m_AverageTime += frame.Time;
		m_Stats.m_RangeLow = fmin(m_Stats.m_RangeLow, frame.Time);
		m_Stats.m_RangeHigh = fmax(m_Stats.m_RangeHigh, frame.Time);
	}
	m_Stats.m_OverallFPS = double(m_Stats.Frames.size()) / total_time;
	m_Stats.m_AverageTime /= (double)m_Stats.Frames.size();

	if (fillallframes)
	{
		for (auto &frame : m_Stats.Frames)
		{
			for (auto &child : frame.ChildEvents)
			{
				child.Parent = &frame;
				FillOutFrame(frame.Time, frame.Time, &child);
			}
		}
	}

	return m_Stats;
}

void ProfileMcGee::FillOutFrame(double frametime, double parenttime, TimedEvent *event)
{
	double this_time = event->Time;
	event->PercentOfFrame = this_time / frametime;
	if (parenttime)
		event->PercentOfParent = this_time / parenttime;
	else
		event->PercentOfParent = 1.0;

	for (auto &child : event->ChildEvents)
	{
		child.Parent = event;
		FillOutFrame(frametime, this_time, &child);
	}
}


constexpr int nameSpace = 20;
constexpr int maxCharacters = 100;
constexpr char indicatorChar = '#';
constexpr char spaceChar = ' ';
constexpr char indentChar = ' ';
constexpr size_t indentSize = 1; // number of spaces indent_char takes up

void PrintEvent(std::ofstream& file, const TimedEvent& e, int indent)
{
	int64_t namePadding = indent * indentSize + nameSpace - e.Name.length();
	std::string realName = e.Name;
	if (namePadding < 0)
	{
		realName = realName.substr(namePadding * -1, realName.length() + namePadding);
		namePadding = 0;
	}
	int myMax = (maxCharacters - realName.length() - namePadding);
	int numIndicators = (int)roundf(e.PercentOfFrame * (float)myMax);
	int numSpaces = myMax - numIndicators;
	file << std::string(namePadding, indentChar) << realName << "-" << std::string(numIndicators, indicatorChar) << std::string(numSpaces, spaceChar) << "| " << e.TimeString();
	if (e.Aggregate)
		file << " [Aggregate of " << e.Aggregations + 1 << "]";
	file << std::endl;
	for (int i = 0; i < e.ChildEvents.size(); ++i)
		PrintEvent(file, e.ChildEvents[i], indent + 1);
}

void FindLeafEventsLongerThan(std::vector<const TimedEvent*>& vec, double minTime, const TimedEvent& e)
{
	if (e.ChildEvents.empty() && e.Time > minTime)
		vec.push_back(&e);

	for (int i = 0; i < e.ChildEvents.size(); ++i)
		FindLeafEventsLongerThan(vec, minTime, e.ChildEvents[i]);
}

void WriteFrameToStream(std::ofstream& file, const TimedEvent& frame)
{
	file << "Frame took " << frame.Time * 1000 << "ms " << std::endl;

	// Print highest contributors first
	std::vector<const TimedEvent*> highestContributors{};
	double minTime = 0.001;
	FindLeafEventsLongerThan(highestContributors, minTime, frame);

	std::sort(highestContributors.begin(), highestContributors.end(), [](const const TimedEvent*& a, const const TimedEvent*& b) { return a->Time < b->Time; });

	file << "Highest Contributors (child-less events with time > 1ms):" << std::endl;
	for (int i = 0; i < highestContributors.size(); ++i)
	{
		int charsUsed = 0;
		const auto& contrib = highestContributors[i];
		file << '\t';
		std::vector<std::string> names;
		for (auto e = contrib; e; e = e->Parent)
			names.push_back(e->Name);

		for (auto it = names.rbegin(); it != names.rend(); it++)
		{
			file << *it;
			charsUsed += it->length();
			if (it + 1 != names.rend())
			{
				file << "-";
				charsUsed += 1;
			}
			else
			{
				file << ": ";
				charsUsed += 2;
			}
		}

		int targetPadding = 80;
		int neededPadding = targetPadding - charsUsed;
		if (neededPadding < 0)
			neededPadding = 0;

		file << std::string(neededPadding, ' ');


		file << contrib->TimeString() << std::endl;
	}
	file << ". . . . END HIGH CONTRIBUTORS . . . ." << std::endl;

	int indent = 0;
	std::string realName = frame.Name;
	int64_t namePadding = (nameSpace + indent * indentSize) - frame.Name.length();
	if (namePadding < 0)
	{
		realName = realName.substr(namePadding * -1, realName.length() + namePadding);
		namePadding = 0;
	}

	int myMax = (maxCharacters - realName.length() - namePadding);
	int num_indicators = (int)roundf(frame.PercentOfFrame * (float)myMax);
	int num_spaces = myMax - num_indicators;

	file << std::string(namePadding, ' ') << frame.Name << "-" << std::string(num_indicators, indicatorChar) << std::string(num_spaces, spaceChar) << "| " << frame.TimeString() << std::endl;

	for (int j = 0; j < frame.ChildEvents.size(); ++j)
		PrintEvent(file, frame.ChildEvents[j], 1);

	file << "---------------------------------------------------------------------------------------" << std::endl;
}

void BigBoiStats::WriteToFile(const BigBoiStats& stats, std::string filename, float frameTimeThreshold)
{

	std::ofstream file{ filename, std::ios::binary };

	if (file.good())
	{

		file << "------------ Profile Information -------------" << std::endl;
		if (frameTimeThreshold > 0.f)
		{
			file << "Long frames only (>" << frameTimeThreshold << "ms)" << std::endl;
			auto longFrames = std::vector<std::reference_wrapper<const TimedEvent>>();

			for (const auto& frame : stats.Frames)
			{
				if ((frame.Time * 1000.f) > frameTimeThreshold)
					longFrames.push_back(std::cref(frame));
			}

			std::sort(longFrames.begin(), longFrames.end(), [](std::reference_wrapper<const TimedEvent> lhs, std::reference_wrapper<const TimedEvent> rhs) { return lhs.get().Time > rhs.get().Time; });
			for (const auto& frame : longFrames)
			{
				WriteFrameToStream(file, frame);
			}
		}
		else
		{
			file << "All Frames" << std::endl;
			for (int i = 0; i < stats.Frames.size(); ++i)
			{
				const auto& frame = stats.Frames[i];

				WriteFrameToStream(file, frame);
			}
		}
		file << "------------ End Profile Information ------------" << std::endl;
	}
}
#endif

