#pragma once

#include "Helpers/DebugHelper.h"
#include "Helpers/SDLHelper.h"

#include "Systems/SDLWrapping/SDLWrappers.h"
#include "Drawing/Image.h"
#include "Math/floaty.h"

struct TextDrawing : Drawing::SDLImage
{
	TextDrawing(CommonResources *resources, Stringy text, Stringy fontrefname, ConfigColorTemp color) : SDLImage({ resources, fontrefname + ':' + text }), Text(std::move(text)), Color(color(resources)), FontName(std::move(fontrefname)) { Update(); }
	~TextDrawing() {}
	
	void Update() noexcept;
	inline void SetFontName(Stringy fontname) { FontName = std::move(fontname); Update(); }
	inline void SetColor(SDL_Color color) { Color = color; Update(); }
	inline void SetColor(Stringy colorname) { Color = std::move(colorname); Update(); }
	inline void SetText(Stringy text) { Text = std::move(text); Update(); }
	inline void SetColor(ConfigColor color) { Color = color; Update(); }

	inline const Stringy& GetText() const noexcept { return Text; }

protected:
	Stringy FontName;
	Stringy Text;
	ConfigColor Color;
	bool NeedUpdate{ false };
};

floaty2 CharacterPosition(const SDLW::Font& font, std::string string, size_t index, float linespacing = 0.f, float wraplength = 0.f);