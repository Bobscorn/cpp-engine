#pragma once

#include "Helpers/DebugHelper.h"

#include "Systems/SDLWrapping/SDLWrappers.h"
#include "Drawing/Image.h"

#ifdef __linux__
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL.h>
#else
#include <SDL_ttf.h>
#include <SDL.h>
#endif

struct TextDrawing : Drawing::SDLImage
{
	TextDrawing(CommonResources *resources, Stringy text, Stringy fontname, ConfigColorTemp color) : SDLImage({ resources, fontname + ':' + text }), Text(text), Color(color(resources)), FontName(fontname) { Update(); }
	~TextDrawing() {}
	
	void Update() noexcept;
	inline void SetFontName(Stringy fontname) { FontName = fontname; Update(); }
	inline void SetColor(SDL_Color color) { Color = color; Update(); }
	inline void SetColor(Stringy colorname) { Color = colorname; Update(); }
	inline void SetText(Stringy text) { Text = text; Update(); }
	inline void SetColor(ConfigColor color) { Color = color; Update(); }

	inline Stringy GetText() const noexcept { return Text; }

protected:
	Stringy FontName;
	Stringy Text;
	ConfigColor Color;
	bool NeedUpdate{ false };
};

floaty2 CharacterPosition(const SDLW::Font& font, std::string string, size_t index, float linespacing = 0.f, float wraplength = 0.f);