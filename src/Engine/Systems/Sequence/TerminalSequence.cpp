#include "TerminalSequence.h"

#include "Helpers/MathHelper.h"

#include "Systems/Input/Config1.h"
#include "Drawing/TextDrawing.h"
#include "Drawing/Graphics2D.h"

const char *Sequence::Loading2TerminalElement::LoadSymbol = u8"█";
const char *Sequence::Loading2TerminalElement::NonLoadSymbol = u8"#";
const char *Sequence::Loading2TerminalElement::LoadingLoadBarSymbol = "-";

double Sequence::TerminalSequencev2::Apply(double localtime)
{
	double tmp = InternalApply(localtime);

	return fmax(0.0, tmp);
}

void Sequence::TerminalSequencev2::EnsureFormat()
{
	mResources->UIConfig->RegisterNewTextFormat("Terminal", { "NotoSansDisplay-Regular.ttf", 12 });
}


void Sequence::TerminalSequencev2::EnsureBrush()
{
	mResources->UIConfig->RegisterNewBrush("Terminal", { 200, 200, 200, 255 });
}


void Sequence::TerminalSequencev2::DrawTheText(std::string amount)
{
	if (amount.size())
	{
		glClearColor(0.f, 0.f, 0.f, 1.f);
		glClear(GL_COLOR_BUFFER_BIT);

		// Draw Tex
		TextDrawing DrawnText{ mResources, amount, "Terminal", "Terminal" };
		mResources->Ren2->SetTransform(Matrixy2x3::Identity());

		PointRect rect;
		floaty2 topleft = floaty2::TLtoGL({ 25.f, 25.f }, { *mResources->HalfWindowWidth, *mResources->HalfWindowHeight });
		floaty2 bottomright = floaty2::TLtoGL({ 25.f + (float)DrawnText.GetWidth(), 25.f + (float)DrawnText.GetHeight() }, { *mResources->HalfWindowWidth, *mResources->HalfWindowHeight });
		rect.left = topleft.x;
		rect.top = topleft.y;
		rect.bottom = bottomright.y;
		rect.right = bottomright.x;

		mResources->Ren2->DrawImage(&DrawnText, rect);

		SDLW::Font *font = mResources->UIConfig->GetTextFormat("Terminal");
		if (!font)
			return;

		floaty2 pos = CharacterPosition(*font, amount, amount.size() - 1, 0.f);

		if (DisplayCursor)
		{
			if (amount.size() != LastStringSize)
				DrawCursor({ ceilf(pos.x) + 26.f, ceilf(pos.y) + 27.f });
			else
				DrawFlickerCursor({ ceilf(pos.x) + 26.f, ceilf(pos.y) + 27.f });
		}
	}
	LastStringSize = amount.size();
}


void Sequence::TerminalSequencev2::DrawCursor(floaty2 here)
{
	floaty2 topleft = floaty2::TLtoGL(here, { *mResources->HalfWindowWidth, *mResources->HalfWindowHeight });
	float fontsize = mResources->UIConfig->GetTextFormat("Terminal")->GetDesc().size;
	floaty2 bottomright = { topleft.x + fontsize * 0.66f, topleft.y - fontsize };

	mResources->Ren2->FillRectangle(topleft, bottomright, GetBrush());
}


void Sequence::TerminalSequencev2::DrawFlickerCursor(floaty2 here)
{
	CaretTimer.Tick();
	double time = CaretTimer.TotalTime();
	double rem = fmod(time, 0.5);

	if (rem < 0.25)
	{
		DrawCursor(here);
	}
}

double Sequence::TerminalSequencev2::InternalApply(double deltatime)
{
	std::string tmp;

	double dtime = deltatime - LastTime;
	while (dtime > 0.0)
	{
		if (CurrentIndex < Elements.size())
		{
			double tmpd = Elements[CurrentIndex]->GetTextString(dtime, tmp);
			if (tmpd > 0.0)
			{
				LastTime += dtime - tmpd;
				++CurrentIndex;
				CompletedString += tmp;
				tmp.clear();
			}
			dtime = tmpd;
		}
		else
		{
			DrawTheText(CompletedString);
			return dtime;
		}
	}

	DrawTheText(CompletedString + tmp);
	return dtime;
}


double Sequence::NormalTextTerminalElement::GetTextString(double deltatime, std::string & out)
{
	out = GetCulledString(deltatime);
	return fmax(0.0, deltatime - duration);
}


size_t Sequence::NormalTextTerminalElement::GetIndexTo(double time)
{
	double percentage = fmin(1.0, time / duration);

	size_t index = (size_t)(Omaewamoushindeiru.size() * percentage);

	return index;
}

std::string Sequence:: NormalTextTerminalElement::GetCulledString(double time)
{
	return Omaewamoushindeiru.substr(0ul, GetIndexTo(time));
}


double Sequence::PauseTerminalElement::GetTextString(double deltatime, std::string & out)
{
	out.clear();
	return fmax(0.0, deltatime - Duration);
}


double Sequence::LoadingTerminalElement::GetTextString(double deltatime, std::string & out)
{
	Terminal->DisableCursor();
	if (deltatime >= Duration)
		out.clear();
	else
	{
		double FullChange = 4.0 * ChangeInterval;
		size_t thing = (size_t)(deltatime / FullChange);
		double timmy = deltatime - thing * FullChange;
		unsigned int ass = (unsigned int)(ceil(timmy / ChangeInterval));
		if (ass <= 1u)
			out = "/";
		else if (ass == 2u)
			out = "-";
		else if (ass == 3u)
			out = "\\";
		else
			out = "|";
	}
	double remainder = fmax(0.0, deltatime - Duration);
	if (remainder)
		Terminal->EnableCursor();
	return remainder;
}


double Sequence::Loading2TerminalElement::GetTextString(double deltatime, std::string & out)
{
	if (deltatime < TextDuration)
	{
		Terminal->EnableCursor();
		out = std::string("[" + GetLoadLoadSymbols(numloadingsymbols) + "] - 00%").substr(0ul, GetTextIndex(deltatime));
	}
	else
	{
		Terminal->DisableCursor();
		double loadtime = deltatime - TextDuration;
		if (loadtime >= Duration)
			out = "[" + GetLoadSymbols(numloadingsymbols) + "] - 100%";
		else
		{
			double percentage = loadtime / Duration;
			size_t LoadSymbolCount = (size_t)(percentage * numloadingsymbols);
			out = "[" + GetLoadSymbols(LoadSymbolCount) + GetNonLoadSymbols(numloadingsymbols - LoadSymbolCount) + "] - " + std::to_string(size_t(percentage * 100)) + "%";
		}
	}
	double remainder = fmax(0.0, deltatime - (TextDuration + Duration));
	if (remainder)
		Terminal->EnableCursor();
	return remainder;
}

size_t Sequence::Loading2TerminalElement::GetTextIndex(double time)
{
	double percent = time / TextDuration;
	return (size_t)(percent * (numloadingsymbols + 9ul));
}

std::string Sequence:: Loading2TerminalElement::GetNonLoadSymbols(size_t count)
{
	std::string out;
	for (size_t i = count; i-- > 0; )
		out += NonLoadSymbol;
	return out;
}

std::string Sequence:: Loading2TerminalElement::GetLoadSymbols(size_t count)
{
	std::string out;
	for (size_t i = count; i-- > 0; )
		out += LoadSymbol;
	return out;
}

std::string Sequence:: Loading2TerminalElement::GetLoadLoadSymbols(size_t count)
{
	std::string out;
	for (size_t i = count; i-- > 0; )
		out += LoadingLoadBarSymbol;
	return out;
}
