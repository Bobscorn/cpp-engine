#include "SDLHelper.h"

#include "Systems/Input/Config1.h"

ConfigColor::ConfigColor(Config1::UIConfig * config, Stringy colorname) : 
	Config(config), 
	ColorName(colorname), 
	Color({ 0, 0, 0, 0 }),
	UseUI(true) 
{}

bool ConfigColor::Update()
{
	if (UseUI)
	{
		SDL_Color tmpcolor = Config->GetBrush(ColorName);
		bool tmp = Color != tmpcolor;
		NewColor = Color = tmpcolor;
		return tmp;
	}
	else
	{
		bool tmp = Color != NewColor;
		Color = NewColor;
		return tmp;
	}
}

ConfigColor & ConfigColor::operator=(SDL_Color color) noexcept
{
	SetColor(color);
	return *this;
}

ConfigColor & ConfigColor::operator=(Stringy colorname) noexcept
{
	SetColorName(colorname);
	return *this;
}

ConfigColor::operator SDL_Color()
{
	if (UseUI)
		return Config->GetBrush(ColorName);
	else
		return Color;
}

PointRect::PointRect(const DimensionRect & rect) : left(rect.x), top(rect.y), right(rect.x + rect.w), bottom(rect.y + rect.h)
{
}

PointRect::operator DimensionRect()
{
	return { x1, y1, x2 - x1, y2 - y1 };
}

ConfigColor ConfigColorTemp::operator()(CommonResources *resources)
{
	return operator()(resources->UIConfig);
}
