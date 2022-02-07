//#include "Thing.h"
#include "BrushHelper.h"

#include "Helpers/TransferHelper.h"

#include "../Systems/Input/Config1.h"

/*
ID2D1Brush * Brush::Brush::operator()(CommonResources * resources)
{
	if (spec == CUSTOM)
		return *custombrush;

	switch (spec)
	{
	case PRIMARY_BRUSH:
		return resources->UIConfig->GeneralBrush->GetBrush();
	case SECONDARY_BRUSH:
		return resources->UIConfig->GeneralAlternative->GetBrush();
	case ACCENT_BRUSH:
		return resources->UIConfig->Accent->GetBrush();
	case TEXT_BRUSH:
		return resources->UIConfig->Text->GetBrush();
	case TEXT_HOVER_BRUSH:
		return resources->UIConfig->TextHover->GetBrush();
	case BUTTON_FILL_BRUSH:
		return resources->UIConfig->ButtonDefault->GetBrush();
	case BUTTON_HOVER_FILL_BRUSH:
		return resources->UIConfig->ButtonHover->GetBrush();
	case TOOLTIP_BRUSH:
		return resources->UIConfig->TooltipBrush->GetBrush();
	}

	return nullptr;
}
*/