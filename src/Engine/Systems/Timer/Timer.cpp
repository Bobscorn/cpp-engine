//#include "Thing.h"
#include "Timer.h"

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#define QueryFreq(x) QueryPerformanceFrequency((LARGE_INTEGER*)&x)
#define QueryCount(x) QueryPerformanceCounter((LARGE_INTEGER*)&x)
#else
#ifdef __linux__
#include <SDL2/SDL_timer.h>
#else
#include <SDL_timer.h>
#endif
#define QueryFreq(x) x = SDL_GetPerformanceFrequency()
#define QueryCount(x) x = SDL_GetPerformanceCounter()
#endif

GameTimer::GameTimer() : Timer()
{
	Uint64 countsPerSec;
	QueryFreq(countsPerSec);
	m_SecondsPerCount = 1.0 / (double)countsPerSec;
}

void GameTimer::Tick()
{
	// Do nothing whilst paused
	if (m_Stopped) {
		m_DeltaTime = 0;
		return;
	}
	// Get the time this frame
	Uint64 currTime;
	QueryCount(currTime);
	m_CurrTime = currTime;
	m_DeltaTime = (m_CurrTime - m_PrevTime) * m_SecondsPerCount;
	// For next framt
	m_PrevTime = m_CurrTime;
	// Delta time might be negative due do processor going into power saver mode
	if (m_DeltaTime < 0.0) {
		m_DeltaTime = 0.0;
	}
}

double GameTimer::DeltaTime() const
{
	return m_DeltaTime;
}

void GameTimer::Reset()
{
	Uint64 currTime;
	QueryCount(currTime);
	m_BaseTime = currTime;
	m_PrevTime = currTime;
	m_StopTime = 0;
	m_PausedTime = 0;
	m_Stopped = false;
}

void GameTimer::Stop()
{
	// Do nothing if its already stopped
	if (m_Stopped)
	{
		return;
	}
	Uint64 currTime;
	QueryCount(currTime);
	// Save the time it stopped at
	m_StopTime = currTime;
	m_Stopped = true;
}

void GameTimer::Start()
{
	if (!m_Stopped)
	{
		return;
	}
	Uint64 startTime;
	QueryCount(startTime);
	// Accumulate Paused time
	m_PausedTime += (startTime - m_StopTime);
	// Because Previous time is not set when timer is paused
	// Previous time is now startTime
	m_PrevTime = startTime;
	// Not stopped any longer
	m_StopTime = 0;
	m_Stopped = false;
}

double GameTimer::TotalTime() const
{
	// Comments here explaining total unpaused time when timer is stopped (wow great explaining, although it is sorta self-explanatory)
	if (m_Stopped)
	{
		return (float)(((m_StopTime - m_PausedTime) - m_BaseTime) * m_SecondsPerCount);
	}
	// Otherwise....
	// More Comments here explaining total unpaused time when timer isn't stopped
	else
	{
		return (float)(((m_CurrTime - m_PausedTime) - m_BaseTime) * m_SecondsPerCount);
	}
}

bool GameTimer::Stopped() const
{
	return m_Stopped;
}

#pragma warning(suppress:4100)
IntervalTimer::IntervalTimer(float interval) : Timer()
{
	Uint64 countsPerSec;
	QueryFreq(countsPerSec);
	m_SecondsPerCount = 1.0 / (double)countsPerSec;
}

void IntervalTimer::Tick()
{
	if (m_Stopped || !m_Interval)
	{
		m_DeltaTime = 0;
		return;
	}
	// Calculate Delta time
	Uint64 currTime;
	QueryCount(currTime);
	m_CurrTime = currTime;
	m_DeltaTime = (m_CurrTime - m_PrevTime) * m_SecondsPerCount;
	if (m_DeltaTime > m_Interval)
	{
		if (!targetFunction)
		{
			return;
		}
		// Execute the target function the number of times DeltaTime fits into the interval
		// The remainder is saved into Previous time so no time is lost
		for (int i = (int)(m_DeltaTime / m_Interval); i > 0; --i) {
			targetFunction();
		}
		// Save Current time minus the remainder of Delta time to
		// prevent it from skipping any time, 
		// minus 1 if fractional value is > 0.5, minus 0 otherwise
		int remaining = (m_CurrTime - m_PrevTime) % (int)(m_Interval / m_SecondsPerCount);
		m_PrevTime = m_CurrTime - remaining;
	}
}

void IntervalTimer::Reset()
{
	Uint64 currTime;
	QueryCount(currTime);
	// Set base time to time of Reset
	m_BaseTime = currTime;
	m_PrevTime = currTime;
	// Reset stop time, and enable timer
	m_StopTime = 0;
	m_Stopped = false;
	m_Interval = 0;
}

void IntervalTimer::Reset(float interval)
{
	Uint64 currTime;
	QueryCount(currTime);
	// Set base time to time of Reset
	m_BaseTime = currTime;
	m_PrevTime = currTime;
	// Reset stop time, and enable timer
	m_StopTime = 0;
	m_Stopped = false;
	m_Interval = interval;
}

void IntervalTimer::Stop()
{
	// Do nothing if already stopped
	if (m_Stopped)
	{
		return;
	}
	Uint64 currTime;
	QueryCount(currTime);
	m_StopTime = currTime;
	m_Stopped = true;
}

void IntervalTimer::Start()
{
	if (!m_Stopped)
	{
		return;
	}
	Uint64 currTime;
	QueryCount(currTime);
	m_PausedTime += currTime - m_StopTime;
	// Reset Previous time to prevent 'time leak'
	m_PrevTime = currTime;
	// Not Stopped any longer
	m_StopTime = 0;
	m_Stopped = false;
}

