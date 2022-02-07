//#include "Thing.h"
#include "UIPositioner.h"
#include "Helpers/TransferHelper.h"
#include "Helpers/VectorHelper.h"

#include "Systems/Input/Config1.h"

#include "Drawing/Graphics2D.h"

bool UIPositioner::RectangleInfo::Within(floaty2 point)
{
	floaty2 transpoint = WorldToLocal.TransformPoint(point);
	auto &t = LocalBounds;
	return (transpoint.x > t.left
		&& transpoint.y < t.top
		&& transpoint.x < t.right
		&& transpoint.y > t.bottom);
}

void UIPositioner::RectangleInfo::MatrixChanged()
{
}

Recty UIPositioner::RectangleInfo::GetBoundingBox()
{
	floaty2 
		A{ LocalToWorld.TransformPoint({ LocalBounds.left, LocalBounds.top }) }, // TopLeft
		B{ LocalToWorld.TransformPoint({ LocalBounds.right, LocalBounds.top }) }, // TopRight
		C{ LocalToWorld.TransformPoint({ LocalBounds.right, LocalBounds.bottom }) }, // BottomRight
		D{ LocalToWorld.TransformPoint({ LocalBounds.left, LocalBounds.bottom }) }; // Bottom Left

	float 
		LeftMost{ fminf(A.x, fminf(B.x, fminf(C.x, D.x))) },
		TopMost{ fminf(A.y, fminf(B.y, fminf(C.y, D.y))) },
		RightMost{ fmaxf(A.x, fmaxf(B.x, fmaxf(C.x, D.x))) },
		BottomMost{ fmaxf(A.y, fmaxf(B.y, fmaxf(C.y, D.y))) };

	return { LeftMost, TopMost, RightMost, BottomMost };
}

void UIPositioner::RectangleInfo::RecommendBoundingBox(Recty parentspace)
{
	float ScaleX = (parentspace.right - parentspace.left) / (LocalBounds.right - LocalBounds.left), ScaleY = (parentspace.top - parentspace.bottom) / (LocalBounds.top - LocalBounds.bottom);
	LocalToParent = Matrixy2x3::Scale({ ScaleX, ScaleY });

	LocalToParent = LocalToParent * Matrixy2x3::Translation(
		0.5f * ((parentspace.left + parentspace.right) - ScaleX * (LocalBounds.left + LocalBounds.right)), 
		0.5f * ((parentspace.top + parentspace.bottom) - ScaleY * (LocalBounds.bottom + LocalBounds.top))
	);
}

void UIPositioner::RectangleInfo::DebugDraw(CommonResources * resources)
{
	floaty2
		A{ LocalToWorld.TransformPoint({ LocalBounds.left, LocalBounds.top }) }, // TopLeft
		B{ LocalToWorld.TransformPoint({ LocalBounds.right, LocalBounds.top }) }, // TopRight
		C{ LocalToWorld.TransformPoint({ LocalBounds.right, LocalBounds.bottom }) }, // BottomRight
		D{ LocalToWorld.TransformPoint({ LocalBounds.left, LocalBounds.bottom }) }; // Bottom Left

	resources->Ren2->SetTransform(Matrixy2x3::Identity());

	SDL_Color dline = resources->UIConfig->GetBrush("DebugLine");
	resources->Ren2->DrawRectangle(D, C, B, A, dline);
}

bool UIPositioner::CircleInfo::Within(floaty2 point)
{
	return Distance(point, TransformedCenter) < Radius;
}

void UIPositioner::CircleInfo::MatrixChanged()
{
	TransformedCenter = LocalToWorld.TransformPoint({ 0.f, 0.f });
}

void UIPositioner::CircleInfo::RecommendBoundingBox(Recty box)
{
	floaty2 center = { 0.5f * (box.left + box.right), 0.5f * (box.top + box.bottom) };

	float Scale = Radius / fminf(box.right - center.x, box.bottom - center.y);

	LocalToParent = Matrixy2x3::Scale({ Scale, Scale });
	LocalToParent = LocalToParent * Matrixy2x3::Translation(center.x, center.y);
}
