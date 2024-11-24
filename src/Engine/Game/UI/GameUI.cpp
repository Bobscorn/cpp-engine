#include "GameUI.h"

#include "Systems/Requests/Requester.h"
#include "Drawing/Graphics2D.h"

bool UI1I::Buttony::OnMouseLeftClick(floaty2 & coords)
{
	(void)coords;
	mResources->Request->Request(action);
	return true;
}


Debug::DebugReturn UI1I::ButtonyContainer::Initialize()
{
	return true;
}

void UI1I::ButtonyContainer::IBeforeDraw()
{
	if (*mResources->WindowFocused)
	{
		if (*mResources->MouseChangedPosition)
		{
			float MoveX = float(double(mResources->MousePosition->x) * *mResources->InverseWindowWidth);
			float MoveY = float(double(mResources->MousePosition->y) * *mResources->InverseWindowHeight);
			TransX = MoveX * Marginny * 0.125f;
			TransY = MoveY * Marginny * 0.125f;
			UpdateImageTrans();
		}
	}
}

void UI1I::ButtonyContainer::IDraw()
{
	if (Image)
	{
		mResources->Ren2->SetTransform(ImageTrans);
		mResources->Ren2->DrawImage(&Image, { -Image.GetWidth() * 0.5f, -Image.GetHeight() * 0.5f });
	}
}

void UI1I::ButtonyContainer::ComputePosition(Matrixy2x3 accumulated)
{
	UpdateImageScale();

	auto start = GetAlignmentPosition();
	accumulated = accumulated * Matrixy2x3::Translation(start.x, 0.f);
	float y = start.y;
	float maxWidth = 0.f;
	for (auto& button : Buttons)
	{
		button->PreComputePosition();
		float tmpy = y - button->GetHeight();
		button->RecommendBoundingBox({ -button->GetMargin(), y, button->GetFullWidth() - button->GetMargin(), tmpy });
		y = tmpy;
		button->ComputeWorldMatrix(accumulated);
		maxWidth = std::max(button->GetFullWidth(), maxWidth);
	}

	Bounds.x = start.x;
	Bounds.y = y;
	Bounds.z = maxWidth;
	Bounds.w = start.y;
}

void UI1I::ButtonyContainer::UpdateImageScale()
{
	if (Image)
	{
		float DesiredWidth = float(*mResources->WindowWidth) + Marginny * 2.f
			, DesiredHeight = float(*mResources->WindowHeight) + Marginny * 2.f;

		float ActualWidth = (float)Image.GetWidth(), ActualHeight = (float)Image.GetHeight();

		Scale = fmaxf(DesiredWidth / ActualWidth, DesiredHeight / ActualHeight);

		UpdateImageTrans();
	}
}

void UI1I::ButtonyContainer::UpdateImageTrans()
{
	if (Image)
	{
		ImageTrans = Matrixy2x3::Scale({ Scale, Scale });
		ImageTrans = ImageTrans * Matrixy2x3::Translation(TransX, TransY);
	}
}

bool UI1I::ButtonyContainer::Within(floaty2 coords)
{
	return coords.x > Bounds.x &&
		coords.y > Bounds.y &&
		coords.x < Bounds.z&&
		coords.y < Bounds.w;
}

floaty2 UI1I::ButtonyContainer::GetAlignmentPosition()
{
	floaty2 out;
	float margin_size = mResources->UIConfig->GetBigMarginSize();
	if (X_Align == LEFT_ALIGN)
		out.x = -(float)*mResources->HalfWindowWidth + margin_size * 2.f;
	else if (X_Align == RIGHT_ALIGN)
	{
		float max_width = 0.f;
		for (auto& button : Buttons)
			max_width = fmaxf(max_width, button->GetWidth());

		out.x = (float)*mResources->HalfWindowWidth - margin_size * 3.f - max_width;
	}
	else
	{
		float max_width = 0.f;
		for (auto& button : Buttons)
			max_width = fmaxf(max_width, button->GetWidth());
		out.x = -(max_width * 0.5f);
	}
	if (Y_Align == TOP_ALIGN)
		out.x = (float)*mResources->HalfWindowHeight - margin_size * 3.f;
	else if (Y_Align == BOTTOM_ALIGN)
	{
		float TotalHeight{ 0.f };
		for (auto& butt : Buttons)
			TotalHeight += butt->GetHeight();

		out.y = -(float)*mResources->HalfWindowHeight + margin_size * 3.f + TotalHeight;
	}
	else
	{
		float TotalHeight{ 0.f };
		for (auto& butt : Buttons)
			TotalHeight += butt->GetHeight();
		out.y = TotalHeight * 0.5f;
	}
	return out;
}

Debug::DebugReturn UI1I::TitleText::Initialize()
{
	float HalfHeight = 0.5f * (GetFontSize() + GetMargin());
	float Width = GetFullWidth();
	this->LocalBounds.top = HalfHeight;
	this->LocalBounds.bottom = -HalfHeight;
	this->LocalBounds.left = -Width * 0.5f;
	this->LocalBounds.right = Width * 0.5f;
	
	return true;
}

floaty2 UI1I::UIPosition::Transform(floaty2 in, floaty4 parentBounds) const 
{
	floaty2 preAnchor = in + floaty2{ m_HalfBounds.x * -m_Pivot.x, m_HalfBounds.y * -m_Pivot.y };
	floaty2 halfParentDims = floaty2{ 0.5f * (parentBounds.z - parentBounds.x), 0.5f * (parentBounds.w - parentBounds.y) };

	floaty2 anchorOrigin = parentBounds.xy() + halfParentDims + floaty2{ halfParentDims.x * m_Anchor.x, halfParentDims.y * m_Anchor.y };
	return preAnchor + anchorOrigin;
}

floaty4 UI1I::UIPosition::Transform(floaty4 myBounds, floaty4 parentBounds) const 
{
	floaty2 pivotOrigin = floaty2{ m_HalfBounds.x * -m_Pivot.x, m_HalfBounds.y * -m_Pivot.y };
	floaty4 preAnchor = myBounds + floaty4{ pivotOrigin.x, pivotOrigin.y, pivotOrigin.x, pivotOrigin.y };
	floaty2 halfParentDims = floaty2{ 0.5f * (parentBounds.z - parentBounds.x), 0.5f * (parentBounds.w - parentBounds.y) };

	floaty2 anchorOrigin = parentBounds.xy() + halfParentDims + floaty2{ halfParentDims.x * m_Anchor.x, halfParentDims.y * m_Anchor.y };
	return preAnchor + floaty4{ anchorOrigin.x, anchorOrigin.y, anchorOrigin.x, anchorOrigin.y };
}

floaty4 UI1I::UIPosition::GetTransformedBounds(floaty4 parentBounds) const
{
	return Transform(floaty4{ -m_HalfBounds.x, -m_HalfBounds.y, m_HalfBounds.x, m_HalfBounds.y }, parentBounds);
}

void UI1I::SizedUITextBox::IDraw()
{
	mResources->Ren2->SetTransform(this->LocalToWorld);
	
	floaty4 bounds = m_Position.GetTransformedBounds(m_ParentBounds);
	mResources->Ren2->DrawImage(&Text, LocalBounds);
}

void UI1I::UIImage::IDraw()
{
	mResources->Ren2->SetTransform(this->LocalToWorld);

	// Pretend parent bounding box is the window
	floaty4 windowBounds{ -(*mResources->HalfWindowWidth), -(*mResources->HalfWindowHeight), *mResources->HalfWindowWidth, *mResources->HalfWindowHeight};
	floaty4 realBounds = m_Position.GetTransformedBounds(windowBounds);

	mResources->Ren2->DrawImage(m_Image.get(), PointRect{ realBounds.x, realBounds.w, realBounds.z, realBounds.y });
}
