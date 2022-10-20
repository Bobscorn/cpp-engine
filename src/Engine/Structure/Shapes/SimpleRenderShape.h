#pragma once

#include "Drawing/Graphics1.h"

#include "Math/matrix.h"
#include "Drawing/DrawCallReference.h"

#include <memory>

namespace G1I
{
	class SimpleRenderShape : public G1::IShape
	{
	protected:
		Drawing::DrawCallReference m_DrawCall;
		std::shared_ptr<Matrixy4x4> m_DrawCallTrans;

	public:
		SimpleRenderShape(G1::IShapeThings things, Drawing::DrawCallReference drawCall);

		inline virtual void BeforeDraw() override {}
		inline virtual void AfterDraw() override {}

		void SetMatrix(const Matrixy4x4& mat);
	};
}

