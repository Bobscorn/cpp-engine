#pragma once

#include "UI/UI1.h"

namespace Voxel
{
#pragma warning(push)
#pragma warning(disable:4250)
	struct Crosshair : UI1::BlankElement, virtual UI1::UIElement
	{
		Crosshair() : UI1::UIElement() { CalculateCrosshairCentre(); }

		void IDraw() override;

		Stringy GetName() const override;

	protected:

		void CalculateCrosshairCentre();

		floaty2 m_CrosshairCentre;
	};
#pragma warning(pop)
}