#pragma once

struct CommonResources;
/*
namespace Brush
{
	enum BrushSpecifier
	{
		CUSTOM,
		PRIMARY_BRUSH,
		SECONDARY_BRUSH,
		ACCENT_BRUSH,
		TEXT_BRUSH,
		TEXT_HOVER_BRUSH,
		BUTTON_FILL_BRUSH,
		BUTTON_HOVER_FILL_BRUSH,
		TOOLTIP_BRUSH,
	};

	struct Brush
	{
		BrushSpecifier spec;
		ID2D1Brush **custombrush;

		Brush() : spec(PRIMARY_BRUSH), custombrush(nullptr) {};
		Brush(BrushSpecifier b, ID2D1Brush **brush = nullptr) : spec(b), custombrush(brush) {}

		ID2D1Brush *operator() (CommonResources *resources);
	};
}*/