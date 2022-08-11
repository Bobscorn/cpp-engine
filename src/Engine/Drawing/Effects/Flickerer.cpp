#include "Flickerer.h"

#include "Drawing/Graphics2D.h"

float Flicker::Apply(float time)
{
	if (IsOnFlicker(time))
		DoFlicker();
	else if (UnFlicker)
		DoUnFlicker();
	return fmaxf(0.f, time - Duration);
}

void Flicker::DoFlicker()
{
	mResources->Ren2->Clear(FlickerColor);
	DINFO("Flickered");
}

void Flicker::DoUnFlicker()
{
	mResources->Ren2->Clear({ 0, 0, 0, 255 });
	DINFO("Unflickered");
}

bool Flicker::IsOnFlicker(float time)
{
	if (time < 0.f || time > Duration)
		return false;

	// Calculate Intervals


	float FlickerStart{ 0.f };
	for (float i = 0u; i < FlickerCount; ++i)
	{
		if (time > FlickerStart && time < (FlickerStart += FlickerDuration))
			return true;
		FlickerStart += FlickerInterval;
	}

	return false;
}

void TimedFlicker::Apply()
{
	Timmy.Tick();
	Flicker::Apply(float(Timmy.TotalTime()));
}
