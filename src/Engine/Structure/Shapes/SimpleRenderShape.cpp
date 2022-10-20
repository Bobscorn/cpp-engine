#include "SimpleRenderShape.h"

#include "Drawing/DrawCall.h"

G1I::SimpleRenderShape::SimpleRenderShape(G1::IShapeThings things, Drawing::DrawCallReference drawCall)
	: IShape(things)
	, m_DrawCall(std::move(drawCall))
	, m_DrawCallTrans(nullptr)
{
	if (auto* pCall = m_DrawCall.GetDrawCall())
	{
		m_DrawCallTrans = pCall->Matrix;
	}
}

void G1I::SimpleRenderShape::SetMatrix(const Matrixy4x4& mat)
{
	if (m_DrawCallTrans)
		*m_DrawCallTrans = mat;
}
