#pragma once

#include "Systems/Timer/Timer.h"

#ifdef EC_PROFILE
#ifdef NDEBUG
#define PROFILE_PUSH(x) mResources->Profile->Push(x)
#define PROFILE_PUSH_WITH(prof, x) prof->Push(x)
#define PROFILE_PUSH_AGG(x) mResources->Profile->PushAggregate(x)
#define PROFILE_PUSH_AGG_WITH(prof, x) prof->PushAggregate(x)
#define PROFILE_POP() mResources->Profile->Pop()
#define PROFILE_POP_WITH(prof) prof->Pop()

#define PROFILE_EVENT(x, agg) ProfileEvent{ mResources->Profile, x, agg }
#define PROFILE_EVENT_WITH(prof, x, agg) ProfileEvent{ prof, x, agg }
#else
#include "DebugHelper.h"
#define PROFILE_PUSH(x) mResources->Profile->Push(x, __FUNCTION_NAME__, __LINE__)
#define PROFILE_PUSH_WITH(prof, x) prof->Push(x, __FUNCTION_NAME__, __LINE__)
#define PROFILE_PUSH_AGG(x) mResources->Profile->PushAggregate(x, __FUNCTION_NAME__, __LINE__)
#define PROFILE_PUSH_AGG_WITH(prof, x) prof->PushAggregate(x, __FUNCTION_NAME__, __LINE__)
#define PROFILE_POP() mResources->Profile->Pop(__FUNCTION_NAME__, __LINE__)
#define PROFILE_POP_WITH(prof) prof->Pop(__FUNCTION_NAME__, __LINE__)

#define PROFILE_EVENT(x, agg) ProfileEvent{ mResources->Profile, x, agg, __FUNCTION_NAME__, __LINE__ }
#define PROFILE_EVENT_WITH(prof, x, agg) ProfileEvent{ prof, x, agg, __FUNCTION_NAME__, __LINE__ }
#endif // _DEBUG
#else
#define PROFILE_PUSH(x) ((int)0)
#define PROFILE_POP() ((int)0)
#define PROFILE_PUSH_WITH(prof, x) ((int)0)
#define PROFILE_PUSH_AGG(x) ((int)0)
#define PROFILE_PUSH_AGG_WITH(prof, x) ((int)0)
#define PROFILE_POP_WITH(prof) ((int)0)

#define PROFILE_EVENT(x, agg) ((int)0)
#define PROFILE_EVENT_WITH(prof, x, agg) ((int)0)
#endif // EC_PROFILE

#ifdef EC_PROFILE
#include <string>
#include <vector>
#include <stack>
#include <cstdio>

struct TimedEvent
{
#ifdef _DEBUG
	TimedEvent(std::string name = "", double starttime = 0.0, TimedEvent *parent = nullptr, const char * signature = "", size_t line = 0, bool aggregate = false) : Name(name), StartTime(starttime), Parent(parent), Signature(signature), Line(line), Aggregate(aggregate ? aggregate : (Parent ? Parent->Aggregate : false)) {}
#else
	TimedEvent(std::string name = "", double starttime = 0.0, TimedEvent *parent = nullptr, bool aggregate = false) : Name(name), StartTime(starttime), Parent(parent), Aggregate(aggregate ? aggregate : (Parent ? Parent->Aggregate : false)) {}
#endif // _DEBUG
	std::string Name;

	double PercentOfFrame = 0.0;
	double PercentOfParent = 0.0;
	double Time = 0.0;

	double StartTime = 0.0;
	TimedEvent *Parent = nullptr;
	double EndTime = 0.0;
	std::vector<TimedEvent> ChildEvents;

#ifdef _DEBUG
	const char *Signature;
	size_t Line;

#endif
	bool Aggregate = false;
	size_t Aggregations = 0;

	bool operator==(const TimedEvent& other) const;
	inline bool operator!=(const TimedEvent& other) const { return !(*this == other); }

	inline std::string TimeString() const
	{
		char info_buffer[90] = { 0 };
		auto buf_len = snprintf(info_buffer, sizeof(info_buffer), "%4.2fms - %4.2f%% of Frame - %4.2f%% of Parent", Time * 1000.0, PercentOfFrame * 100.0, PercentOfParent * 100.0);
		return std::string(info_buffer, info_buffer + buf_len);
	}

	void AggregateChildren();
	void AggregateOntoThis(TimedEvent& other, TimedEvent& parent);
};

struct BigBoiStats
{
	std::vector<TimedEvent> Frames;
	
	double m_AverageTime = 0.0;
	double m_OverallFPS = 0.0;
	double m_RangeLow = 0.0;
	double m_RangeHigh = 0.0;

	TimedEvent *LongestFrame = nullptr;
	TimedEvent *ShortestFrame = nullptr;

	static void WriteToFile(const BigBoiStats& stats, std::string filename = "profile.txt");
};

struct ProfileOptions
{
	int gotem = 1;
};

struct ProfileMcGee
{
	ProfileMcGee(ProfileOptions options);

#ifdef _DEBUG
	void Push(std::string name, const char *func, size_t line);
	void PushAggregate(std::string name, const char* func, size_t line);
	void Pop(const char *func, size_t line);
#else
	void Push(std::string name);
	void PushAggregate(std::string name);
	void Pop();
#endif

	void BeginFrame();
	TimedEvent EndFrame();

	void Start();
	const BigBoiStats &Finish(bool fillallframes = false);

	const BigBoiStats &Query(bool fillallframes = false); // Deprecated and I'm lazy
protected:

	void FillOutFrame(double frametime, double parenttime, TimedEvent *event);

	ProfileOptions m_Options;
	GameTimer m_TimerBoi;
	BigBoiStats m_Stats;
	TimedEvent m_WorkingFrame;
	TimedEvent *m_CurrentFrame = nullptr;
	TimedEvent *m_CurrentChild = nullptr;
	bool m_Running = false;
};

struct ProfileEvent
{
	ProfileMcGee* Profiler;

#ifdef _DEBUG
	const char* func;
	int line;

	ProfileEvent(ProfileMcGee* p, std::string eventName, bool agg, const char* func, int line) : Profiler(p), func(func), line(line) { if (agg) p->PushAggregate(eventName, func, line); else p->Push(eventName, func, line); }
	~ProfileEvent() { Profiler->Pop(func, line); }
#else
	ProfileEvent(ProfileMcGee* p, std::string eventName, bool agg) : Profiler(p) { if (agg) p->PushAggregate(eventName); else p->Push(eventName); }
	~ProfileEvent() { Profiler->Pop(); }
#endif
};
#endif