#pragma once

#ifdef __linux__
#include <SDL2/SDL_timer.h>
#else
#include <SDL_timer.h>
#endif

class Timer {
public:
	void virtual Reset() = 0;
	void virtual Stop() = 0;
	void virtual Start() = 0;
	void virtual Tick() = 0;

	double virtual TotalTime() const = 0;

protected:
	Timer() : m_SecondsPerCount(0.0), m_DeltaTime(-1.0),
		m_BaseTime(0), m_PausedTime(0), m_PrevTime(0),
		m_CurrTime(0), m_Stopped(false) {}
	// Used to convert whatever unit QueryPerformanceCounter returns into seconds
	double m_SecondsPerCount;
	double m_DeltaTime;
	// The time the timer was started or last Reset
	Uint64 m_BaseTime;
	Uint64 m_PausedTime;
	Uint64 m_StopTime;
	Uint64 m_PrevTime;
	Uint64 m_CurrTime;
	bool m_Stopped;
};

class GameTimer : public Timer {
public:
	GameTimer();
	double GameTime() const;
	double DeltaTime() const;

	void Reset() override;
	void Stop() override;
	void Start() override;
	void Tick() override;
	double TotalTime() const override;
	bool Stopped() const;
private:
};


class IntervalTimer : public Timer {
public:
	IntervalTimer(float interval);
	void Reset(float interval);

	void virtual Reset();
	void virtual Stop();
	void virtual Start();
	void virtual Tick();
private:
	// Repeat Interval
	float m_Interval;
	void(*targetFunction)();
};

