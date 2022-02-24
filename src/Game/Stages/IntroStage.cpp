#include "IntroStage.h"

#include "Systems/Input/Config1.h"
#include "Drawing/TextDrawing.h"
#include "Drawing/Graphics2D.h"

void IntroStage::BeforeDraw()
{
}

void IntroStage::Draw()
{
	if (!IsDone)
	{
		IsDone = Actions.Apply();
		double Opacity = fmax(0.0, 1.0 - (Actions.GetTime() / 4.0));
		mResources->Ren2->FillRectangle({ -*mResources->HalfWindowWidth, *mResources->HalfWindowHeight }, { *mResources->HalfWindowWidth, -*mResources->HalfWindowHeight }, { 0.f, 0.f, 0.f, (float)Opacity });
		if (IsDone)
			DINFO("Intro Finished");
	}
	else
		Manager->ChangeStageTo("Prologue");
}

void IntroStage::AfterDraw()
{
}

double TerminalSequence::Apply(double localtime)
{
	size_t End = GetIndex(localtime);

	std::string Display = Content.substr(0ul, End);

	glClearColor(0.f, 0.f, 0.f, 1.f);
	glClear(GL_COLOR_BUFFER_BIT);

	DrawTheText(Display);
	
	return fmax(0.0, localtime - Duration);
}

void TerminalSequence::EnsureFormat()
{
	mResources->UIConfig->RegisterNewTextFormat("Terminal", { "NotoSans-Regular.ttf", 12.f });
}

void TerminalSequence::EnsureBrush()
{
	mResources->UIConfig->RegisterNewBrush("Terminal", { 0, 204, 0, 255 });
}

size_t TerminalSequence::GetIndex(double localtime) const
{
	double percentage = fmin(1.0, localtime / Duration);

	percentage = percentage * Content.size();

	return (size_t)percentage;
}

void TerminalSequence::DrawTheText(std::string amount) const
{
	if (amount.size())
	{
		floaty2 target = floaty2::TLtoGL({ 25.f, 25.f }, { *mResources->HalfWindowWidth, *mResources->HalfWindowHeight });

		TextDrawing text = { mResources, amount, "Terminal", "Terminal" };

		mResources->Ren2->SetTransform(Matrixy2x3::Identity());
		mResources->Ren2->DrawImage(&text, target);
	}
}

double FadeSequence::Apply(double localtime)
{
	float Opacity = float(localtime / (Duration * Duration));
	
	floaty4 color = { 0.f, 0.f, 0.f, Opacity };
	floaty2 topleft = { -*mResources->HalfWindowWidth, +*mResources->HalfWindowHeight };
	floaty2 bottomright = { +*mResources->HalfWindowWidth, -*mResources->HalfWindowHeight };
	mResources->Ren2->SetTransform(Matrixy2x3::Identity());
	mResources->Ren2->FillRectangle(topleft, bottomright, color);

	return fmax(0.0, localtime - Duration);
}