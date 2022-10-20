#pragma once

namespace Time
{
	using TimeType = float;
	using TimeScaleType = float;

	class Time
	{
	protected:
		TimeType m_TimeSinceBeginning;

		TimeType m_RunningTime;
		TimeType m_UnscaledRunningTime;
		TimeType m_DeltaTime;
		TimeType m_UnscaledDeltaTime;

		TimeType m_FixedDeltaTime;
		TimeType m_UnscaledFixedDeltaTime;

		TimeScaleType m_TimeScale;
	public:
		Time(TimeType timeScale = 1.f, TimeType fixedDeltaTime = 1.f / 60.f);

		
		void SetRunningTime(TimeType time); // Sets both Real Running Time and Running Time
		void SetScaledRunningTime(TimeType time); // Sets only scaled running time

		void SetNextDeltaTime(TimeType realDeltaTime); // Increments m_RunningTime, m_UnscaledRunningTime, and updates m_DeltaTime and m_UnscaledDeltaTime;

		void SetTimeScale(TimeScaleType scale);
		void SetFixedDeltaTime(TimeType fixedDeltaTime); // Sets unscaled fixed delta time and scaled fixed delta time, scaled fixed delta time is scaled by time scale

		inline const float& GetTimeScale() const { return m_TimeScale; }
		inline const float& GetRunningTime() const { return m_RunningTime; }
		inline const float& GetUnscaledRunningTime() const { return m_UnscaledRunningTime; }
		inline const float& GetDeltaTime() const { return m_DeltaTime; }
		inline const float& GetUnscaledDeltaTime() const { return m_UnscaledDeltaTime; }
		inline const float& GetFixedDeltaTime() const { return m_FixedDeltaTime; }
		inline const float& GetUnscaledFixedDeltaTime() const { return m_UnscaledFixedDeltaTime; }
	};
}

