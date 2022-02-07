#pragma once

#include "Helpers/TransferHelper.h"

#include "Systems/SDLWrapping/SDLWrappers.h"

#ifdef __linux__
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#else
#include <SDL.h>
#include <SDL_ttf.h>
#endif

#include <string>

struct IFontHolder : virtual FullResourceHolder
{
	IFontHolder() {}
	IFontHolder(Stringy font) : FontName(font) {}

	virtual SDLW::Font *GetBackupFormat() const = 0;
	SDLW::Font *GetFormat() const;
	inline void SetFormat(Stringy Name)
	{
		FontName = Name;
	}
protected:
	Stringy FontName;
};

struct SmallFontHolder : IFontHolder
{
	SmallFontHolder() {}
	SmallFontHolder(Stringy font) : IFontHolder(font) {}

	virtual SDLW::Font *GetBackupFormat() const override;
};

struct NormalFontHolder : IFontHolder
{
	NormalFontHolder() {}
	NormalFontHolder(Stringy font) : IFontHolder(font) {}

	virtual SDLW::Font *GetBackupFormat() const override;
};

struct BigFontHolder : IFontHolder
{
	BigFontHolder() {}
	BigFontHolder(Stringy font) : IFontHolder(font) {}

	virtual SDLW::Font *GetBackupFormat() const override;
};

struct IBrushHolder : virtual FullResourceHolder
{
	IBrushHolder() {}
	IBrushHolder(Stringy brush) : BrushName(brush) {}

	virtual SDL_Color GetBackupBrush() const = 0;
	SDL_Color GetBrush() const;
	inline void SetBrush(Stringy Name)
	{
		BrushName = Name;
	}
protected:
	Stringy BrushName;
};

struct GeneralBrushHolder : IBrushHolder
{
	GeneralBrushHolder() {}
	GeneralBrushHolder(Stringy Name) : IBrushHolder(Name) {}
	virtual SDL_Color GetBackupBrush() const override;
};

struct TextBrushHolder : IBrushHolder
{
	TextBrushHolder() {}
	TextBrushHolder(Stringy Name) : IBrushHolder(Name) {}

	virtual SDL_Color GetBackupBrush() const override;
};

struct IHoverBrush : virtual FullResourceHolder
{
	IHoverBrush() {}
	IHoverBrush(Stringy name) : HoverBrushName(name) {}

	virtual SDL_Color GetBackupHoverBrush() const = 0;
	SDL_Color GetHoverBrush() const;
	inline void SetHoverBrush(Stringy Name)
	{
		HoverBrushName = Name;
	}

protected:
	Stringy HoverBrushName;
};