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

/// Simple structs
struct DimensionRect;

//#pragma warning(disable:4201)
struct PointRect
{
	union
	{
		struct
		{
			float x1, y1, x2, y2;
		};
		struct
		{
			float left, top, right, bottom;
		};
	};

	PointRect() : x1(0.f), y1(0.f), x2(0.f), y2(0.f) {}
	PointRect(float left, float top, float right, float bottom) : left(left), top(top), right(right), bottom(bottom) {}
	PointRect(const DimensionRect &rect);

	operator DimensionRect();
};

struct DimensionRect
{
	DimensionRect() {}
	DimensionRect(float x, float y, float w, float h) : x(x), y(y), w(w), h(h) {}
	DimensionRect(const PointRect &rect) : x(rect.x1), y(rect.y1), w(rect.x2 - rect.x1), h(rect.y2 - rect.y1) {}

	inline DimensionRect &operator=(const PointRect &rect) { x = rect.x1; y = rect.y1; w = rect.x2 - rect.x1; h = rect.y2 - rect.y1; return *this; }

	float x{ 0.f }, y{ 0.f }, w{ 0.f }, h{ 0.f };

	inline operator PointRect() { return { x, y, x + w, y + h }; }
};

typedef DimensionRect Rect;

/// Erroring

inline std::string AsString(const PointRect &rect)
{
	return std::string("PRect(left: ") + std::to_string(rect.left) + ", top: " + std::to_string(rect.top) + ", right: " + std::to_string(rect.right) + ", bottom: " + std::to_string(rect.bottom) + ")";
}

inline std::string AsString(const DimensionRect &rect)
{
	return std::string("DRect(x: ") + std::to_string(rect.x) + ", y: " + std::to_string(rect.y) + ", width: " + std::to_string(rect.w) + ", height: " + std::to_string(rect.h) + ")";
}

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