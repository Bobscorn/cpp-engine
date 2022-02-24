#include "ResourceHolder.h"

#include "Systems/Input/Config1.h"

SDLW::Font * IFontHolder::GetFormat() const
{
	if (auto format = mResources->UIConfig->GetTextFormat(FontName))
	{
		return format;
	}
	return GetBackupFormat();
}

SDLW::Font * SmallFontHolder::GetBackupFormat() const
{
	return mResources->UIConfig->GetTextFormat("Small");
}

SDLW::Font * NormalFontHolder::GetBackupFormat() const
{
	return mResources->UIConfig->GetTextFormat("Normal");
}

SDLW::Font * BigFontHolder::GetBackupFormat() const
{
	return mResources->UIConfig->GetTextFormat("Big");
}

SDL_Color IBrushHolder::GetBrush() const
{
	return mResources->UIConfig->GetBrush(BrushName);
}

SDL_Color  GeneralBrushHolder::GetBackupBrush() const
{
	return mResources->UIConfig->GetBrush("Default");
}

SDL_Color  TextBrushHolder::GetBackupBrush() const
{
	return mResources->UIConfig->GetBrush("Text");
}

