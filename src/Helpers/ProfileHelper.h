#pragma once

#include "Systems/Timer/Timer.h"

#ifdef EC_PROFILE
#ifdef _DEBUG
#include "DebugHelper.h"
#define PROFILE_PUSH(x) mResources->Profile->Push(x, __FUNCTION_NAME__, __LINE__)
#define PROFILE_POP() mResources->Profile->Pop(__FUNCTION_NAME__, __LINE__)
#else
#define PROFILE_PUSH(x) mResources->Profile->Push(x)
#define PROFILE_POP() mResources->Profile->Pop()
#endif // _DEBUG
#else
#define PROFILE_PUSH(x) 
#define PROFILE_POP()
#endif // EC_PROFILE

#ifdef EC_PROFILE
#include <string>
#include <vector>
#include <stack>
#include <cstdio>

struct TimedEvent
{
#ifdef _DEBUG
	TimedEvent(std::string name = "", double starttime = 0.0, TimedEvent *parent = nullptr, const char * signature = "", size_t line = 0) : Name(name), StartTime(starttime), Parent(parent), Signature(signature), Line(line) {}
#else
	TimedEvent(std::string name = "", double starttime = 0.0, TimedEvent *parent = nullptr) : Name(name), StartTime(starttime), Parent(parent) {}
#endif // _DEBUG
#ifdef _DEBUG
	std::string Name;

	double PercentOfFrame = 0.0;
	double PercentOfParent = 0.0;
	double Time = 0.0;

	double StartTime = 0.0;
	TimedEvent *Parent = nullptr;
	double EndTime = 0.0;
	std::vector<TimedEvent> ChildEvents;

	const char *Signature;
	size_t Line;

#else
	std::string Name;
	double StartTime = 0.0;
	TimedEvent *Parent = nullptr;
	double EndTime = 0.0;
	double Time = 0.0;
	std::vector<TimedEvent> ChildEvents;
	
	// Stats only filled in when stats are being queried (currently never filled out)
	double PercentOfParent = 0.0;
	double PercentOfFrame = 0.0;
#endif

	inline std::string TimeString() const
	{
		char info_buffer[90] = { 0 };
		auto buf_len = snprintf(info_buffer, sizeof(info_buffer), "%4.2fms - %4.2f%% of Frame - %4.2f%% of Parent", Time * 1000.0, PercentOfFrame * 100.0, PercentOfParent * 100.0);
		return std::string(info_buffer, info_buffer + buf_len);
	}
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
	void Push(std::string, const char *func, size_t line);
	void Pop(const char *func, size_t line);
#else
	void Push(std::string name);
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
#endif