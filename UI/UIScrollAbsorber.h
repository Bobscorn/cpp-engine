#pragma once

#include "UI1.h"

namespace UI1I
{
	namespace Mechanic
	{
		class UIScrollAbsorber : public virtual UI1::UIElement
		{
			INT AbsorbedScroll;

		public:
			virtual bool OnMouseScroll(floaty2 &coords, INT Scrollage) override;

			inline INT PeakScroll() { return AbsorbedScroll; }
			INT ReleaseScroll();
		};
	}
}