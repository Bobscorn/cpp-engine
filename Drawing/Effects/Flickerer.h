#pragma once

#include <Systems/Execution/ResourceHolder.h>

// Just a simple class that will use ClearRenderTargetView to make the screen flicker
struct Flicker : FullResourceHolder
{
	Flicker(CommonResources *resources) : FullResourceHolder(resources) {}
	Flicker(CommonResources *resources, Uint8 Color[4]) : FullResourceHolder(resources), FlickerColor{ Color[0], Color[1], Color[2], Color[3] } {}
	Flicker(CommonResources *resources, float duration, float count, float flickerduration = DefaultFlicker) : FullResourceHolder(resources), Duration(duration), FlickerCount(count), FlickerDuration(flickerduration) { CalculateInterval(); }

	float Apply(float time);

	inline void SetDoUnFlicker() { UnFlicker = true; }

	// You might be wanting the Apply method instead, this is only public because you might want it for some reason...
	void DoFlicker();
	void DoUnFlicker();

protected:
	bool IsOnFlicker(float time);
	inline void CalculateInterval() { FlickerInterval = ((Duration / FlickerCount) - (FlickerCount * FlickerDuration)); }

	SDL_Color FlickerColor{ 255, 255, 255, 255 };
	float Duration{ DefaultDuration };
	float FlickerCount{ DefaultCount };
	float FlickerDuration{ DefaultFlicker };
	float FlickerInterval{ DefaultInterval };
	bool UnFlicker{ false };

	constexpr static float DefaultDuration = 3.f;
	constexpr static float DefaultCount = 3u;
	constexpr static float DefaultFlicker = 1.f / 60.f;
	constexpr static float DefaultInterval = (DefaultDuration / DefaultCount) - (DefaultCount * DefaultFlicker);
};


#include <Systems/Execution/ResourceHolder.h>
#include <Systems/Timer/Timer.h>

// This is essentially the Flicker class but with built in Timer
struct TimedFlicker : Flicker
{
	TimedFlicker(CommonResources *resources) : Flicker(resources) {}

	void Apply();

private:
	GameTimer Timmy;
	using Flicker::Apply;
};