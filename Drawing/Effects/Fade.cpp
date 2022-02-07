#include "Fade.h"

#include "Drawing/Graphics2D.h"

void SolidFader::Reset()
{
	timmy.Reset();
}

void SolidFader::Start()
{
	timmy.Start();
}

void SolidFader::Stop()
{
	timmy.Stop();
}

bool SolidFader::Fade()
{
	timmy.Tick();
	float opacity = fminf(1.f, (float(timmy.TotalTime()) / Duration));

	floaty2 topleft = { -*mResources->HalfWindowWidth, *mResources->HalfWindowHeight };
	floaty2 bottomright = { -topleft.x, -topleft.y };
	mResources->Ren2->FillRectangle(topleft, bottomright, { 0.f, 0.f, 0.f, opacity });

	return opacity == 1.f;
}
