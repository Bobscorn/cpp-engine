#pragma once

#include "UI1.h"
#include "Systems/Timer/Timer.h"

namespace UI1I
{
	namespace Mechanic
	{
		class UIMouseHold
		{
		protected:
			double PreviousTime = 0.0;
			GameTimer timmy;
		public:
			UIMouseHold() { timmy.Reset(); timmy.Stop(); }
			virtual ~UIMouseHold() {}

			double DeltaTime();

		};

		class UILeftMouseHold : public UIMouseHold, public virtual UI1::UIElement
		{
		public:
			UILeftMouseHold() {}
			virtual ~UILeftMouseHold() {}

			virtual bool OnMouseLeftDown(floaty2 &coords) override;
			virtual bool OnMouseLeftUp(floaty2 &coords) override;
		};
	}
}