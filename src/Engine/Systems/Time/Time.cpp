#include "Time.h"

Time::Time::Time(TimeType timeScale, TimeType fixedDeltaTime)
	: m_TimeSinceBeginning(0.f)
	, m_RunningTime(0.f)
	, m_UnscaledRunningTime(0.f)
	, m_DeltaTime(0.f)
	, m_UnscaledDeltaTime(0.f)
	, m_FixedDeltaTime(fixedDeltaTime)
	, m_UnscaledFixedDeltaTime(fixedDeltaTime * timeScale)
	, m_TimeScale(timeScale)
{
}

void Time::Time::SetRunningTime(TimeType time)
{
	m_RunningTime = m_TimeScale * time;
	m_UnscaledRunningTime = time;
}

void Time::Time::SetScaledRunningTime(TimeType time)
{
	m_RunningTime = time;
}

void Time::Time::SetNextDeltaTime(TimeType realDeltaTime)
{
	m_DeltaTime = realDeltaTime * m_TimeScale;
	m_UnscaledDeltaTime = realDeltaTime;

	m_RunningTime += m_DeltaTime;
	m_UnscaledRunningTime += m_UnscaledDeltaTime;
}

void Time::Time::SetTimeScale(TimeScaleType scale)
{
	m_TimeScale = scale;

	m_FixedDeltaTime = m_UnscaledFixedDeltaTime * scale;
}

void Time::Time::SetFixedDeltaTime(TimeType fixedDeltaTime)
{
	m_FixedDeltaTime = fixedDeltaTime * m_TimeScale;
	m_UnscaledFixedDeltaTime = fixedDeltaTime;
}
