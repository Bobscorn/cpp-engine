#pragma once

#include "Systems/Execution/ResourceHolder.h"
#include "Systems/Timer/Timer.h"

#ifdef __linux__
#include <SDL2/SDL.h>
#else
#include <SDL.h>
#endif

struct SolidFader : FullResourceHolder
{
	SolidFader(CommonResources *resources, float Duration = 2.f, SDL_Color color = { 0, 0, 0, 255 }) : FullResourceHolder(resources), Duration(Duration), Color(color) {}

	void Reset();
	void Start();
	void Stop();
	bool Fade();

protected:
	SDL_Color Color;
	GameTimer timmy;
	float Duration{ 2.f };
};