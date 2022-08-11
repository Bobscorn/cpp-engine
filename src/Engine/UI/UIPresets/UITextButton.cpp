#include "UITextButton.h"

#include "Systems/Input/Config1.h"
#include "Drawing/Graphics2D.h"

Debug::DebugReturn UI1I::UITextButton::Initialize()
{
	float HalfHeight = 0.5f * (GetFontSize() + GetMargin());
	float Width = GetFullWidth();
	this->LocalBounds.top = HalfHeight;
	this->LocalBounds.bottom = -HalfHeight;
	this->LocalBounds.left = -Width * 0.5f;
	this->LocalBounds.right = Width * 0.5f;

	return true;
}

bool UI1I::UITextButton::Receive(Event::FontSizeEvent * e)
{
	(void)e;
	/*
	Text = nullptr;
	if (auto error = Initialize())
	{
	}
	else
	{
		if (error.HasErrors())
			DERROR("Initialize failed when Creating Resources (event) with error: " + error.AsString());
		else
			DERROR("Initialize failed when Creating Resources (event), reason unknown");
	}
	return Events::RelevantEvent;*/
	return true;
}

bool UI1I::UITextButton::Receive(Event::MarginChangeEvent * e)
{
	(void)e;
	this->LocalBounds.bottom = GetMargin() + GetFontSize();

	return true;
}

void UI1I::UITextButton::IDraw()
{
	mResources->Ren2->SetTransform(this->LocalToWorld);
	mResources->Ren2->DrawImage(&Text, LocalBounds);
}

void UI1I::UITextButton::SetTextFormat(Stringy Name)
{
	Text.SetFontName(Name);
	Parent->ChildNeedsUpdate();
}

void UI1I::UITextButtonE::IDraw()
{
	mResources->Ren2->SetTransform(LocalToWorld);
	if (BeingClicked())
	{
		mResources->Ren2->DrawImage(&MouseDownText, LocalBounds);
	}
	else if (BeingHovered(*mResources->UpdateID))
	{
		mResources->Ren2->DrawImage(&HoveredText, LocalBounds);
	}
	else
	{
		mResources->Ren2->DrawImage(&Text, LocalBounds);
	}
}