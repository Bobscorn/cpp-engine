#include "ProfileHelper.h"

#include "DebugHelper.h"

#include <iostream>
#include <fstream>
#include <cstdio>
#include <functional>

constexpr float OtherThreshold = 0.001f;

#ifdef EC_PROFILE
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

void BigBoiStats::WriteToFile(const BigBoiStats& stats, std::string filename)
{
	std::ofstream file{ filename, std::ios::binary };

	int nameSpace = 20;
	int maxCharacters = 100;
	constexpr char indicatorChar = '#';
	constexpr char spaceChar = ' ';
	constexpr char indentChar = ' ';
	constexpr size_t indentSize = 1; // number of spaces indent_char takes up

	std::function<void(const TimedEvent&, int indent)> print_event;
	print_event = [&](const TimedEvent& e, int indent)
	{
		int64_t namePadding = indent * indentSize + nameSpace - e.Name.length();
		std::string realName = e.Name;
		if (namePadding < 0)
		{
			realName = realName.substr(namePadding * -1, realName.length() + namePadding);
			namePadding = 0;
		}
		int myMax = (maxCharacters - realName.length() - namePadding);
		char info_buffer[90];
		auto buf_len = snprintf(info_buffer, sizeof(info_buffer), "%4.2fms - %4.2f%% of Frame - %4.2f%% of Parent", e.Time * 1000.0, e.PercentOfFrame * 100.0, e.PercentOfParent * 100.0);
		int numIndicators = (int)roundf(e.PercentOfFrame * (float)myMax);
		int numSpaces = myMax - numIndicators;
		file << std::string(namePadding, indentChar) << realName << "-" << std::string(numIndicators, indicatorChar) << std::string(numSpaces, spaceChar) << "| " << std::string(info_buffer, info_buffer + buf_len) << std::endl;
		for (int i = 0; i < e.ChildEvents.size(); ++i)
			print_event(e.ChildEvents[i], indent + 1);
	};

	if (file.good())
	{

		file << "------------ Profile Information -------------" << std::endl;
		for (int i = 0; i < stats.Frames.size(); ++i)
		{
			const auto& frame = stats.Frames[i];
			file << "Frame took " << frame.Time * 1000 << "ms " << std::endl;

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

			char info_buffer[90];
			auto buf_len = snprintf(info_buffer, sizeof(info_buffer), "%4.2fms - %4.2f%% of Frame - %4.2f%% of Parent", frame.Time * 1000.0, frame.PercentOfFrame * 100.0, frame.PercentOfParent * 100.0);

			file << std::string(namePadding, ' ') << frame.Name << "-" << std::string(num_indicators, indicatorChar) << std::string(num_spaces, spaceChar) << "| " << std::string(info_buffer, info_buffer + buf_len) << std::endl;

			for (int j = 0; j < frame.ChildEvents.size(); ++j)
				print_event(frame.ChildEvents[j], 1);

			file << "---------------------------------------------------------------------------------------" << std::endl;
		}
		file << "------------ End Profile Information ------------" << std::endl;
	}
}
#endif

