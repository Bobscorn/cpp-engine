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
	float ass = mResources->UIConfig->GetBigMarginSize();

	// Calculate Total Height
	float TotalHeight{ 0.f };
	for (auto& butt : Buttons)
		TotalHeight += butt->GetHeight();

	//float ydisplace = (float(*mResources->WindowHeight) - TotalHeight) * 0.5f;

	accumulated = accumulated * Matrixy2x3::Translation((3.f * ass) - *mResources->HalfWindowWidth, 0.f);// ydisplace);
	float y = TotalHeight * 0.5f;
	for (auto& button : Buttons)
	{
		button->PreComputePosition();
		float tmpy = y - button->GetHeight();
		button->RecommendBoundingBox({ -button->GetMargin(), y, button->GetFullWidth() - button->GetMargin(), tmpy });
		y = tmpy;
		button->ComputeWorldMatrix(accumulated);
	}
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
