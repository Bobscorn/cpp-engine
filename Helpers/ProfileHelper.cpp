#include "ProfileHelper.h"

#include "DebugHelper.h"

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

#endif