#pragma once

#include "Helpers/DebugHelper.h"

#ifdef __linux__
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#else
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#endif

#include <exception>

namespace Config1
{
	struct UIConfig;
}

struct CommonResources;


/// Operators

inline bool operator==(const SDL_Color &a, const SDL_Color &b)
{
	return a.r == b.r
		&& a.g == b.g
		&& a.b == b.b
		&& a.a == b.a;
}

inline bool operator!=(const SDL_Color &a, const SDL_Color &b)
{
	return a.r != b.r
		|| a.g != b.g
		|| a.b != b.b
		|| a.a != b.a;
}

/// Basic Wrappers

struct SimpleSurface
{
	SimpleSurface() : Surface(nullptr) {}
	SimpleSurface(SDL_Surface *surface) : Surface(surface) {}
	SimpleSurface(SimpleSurface &&ss) noexcept : Surface(ss.Surface) { ss.Surface = nullptr; }
	~SimpleSurface() { Delete(); }

	inline SimpleSurface &operator=(SimpleSurface &&ss) noexcept
	{
		Delete();
		Surface = ss.Surface;
		ss.Surface = nullptr;
		return *this;
	}

	inline void Delete()
	{
		if (Surface)
			SDL_FreeSurface(Surface);
		Surface = nullptr;
	}

	inline SDL_Surface *Get() const noexcept
	{
		return Surface;
	}

	inline SDL_Surface *operator->() const noexcept { return Get(); }

	inline SDL_Surface *Release()
	{
		auto tmp = Surface;
		Surface = nullptr;
		return tmp;
	}

	inline explicit operator SDL_Surface*() const noexcept
	{
		return Get();
	}

	inline explicit operator bool() const noexcept { return Get(); }

protected:
	SDL_Surface *Surface;
};


// Special Wrappers

struct ConfigColor
{
	ConfigColor(Config1::UIConfig *config) : Config(config), Color{ 0, 0, 0, 0 } {}
	ConfigColor(Config1::UIConfig *config, Stringy colorname);
	ConfigColor(Config1::UIConfig *config, SDL_Color color) : Config(config), Color(color), UseUI(false) {}

	bool Update();

	inline void SetColor(SDL_Color color)
	{
		NewColor = color;
		UseUI = false;
	}

	inline void SetColorName(Stringy colorname)
	{
		ColorName = colorname;
		UseUI = true;
	}

	ConfigColor &operator=(SDL_Color color) noexcept;
	ConfigColor &operator=(Stringy colorname) noexcept;


	explicit operator SDL_Color();

protected:
	SDL_Color Color;
	SDL_Color NewColor{ 0,0,0,0 };
	Stringy ColorName;
	Config1::UIConfig *Config{ nullptr };
	bool UseUI{ false };
};

struct ConfigColorTemp
{
	ConfigColorTemp() = default;
	ConfigColorTemp(Stringy colorname) : name(colorname), color{ 0, 0, 0, 0 } {}
	ConfigColorTemp(const char *name) : name(name), color{ 0, 0, 0, 0 } {}
	ConfigColorTemp(SDL_Color color) : color(color) {}

	inline ConfigColor operator()(Config1::UIConfig *config)
	{
		if (name.size())
			return ConfigColor(config, name);
		else
			return ConfigColor(config, color);
	}
	ConfigColor operator()(CommonResources *resources);

protected:
	Stringy name;
	SDL_Color color;
};