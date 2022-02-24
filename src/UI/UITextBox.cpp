//#include "Thing.h"
#include "UITextBox.h"

#include "Helpers/TransferHelper.h"

#include "Systems/Input/Config1.h"

/*void UI1I::Drawing::UITextBox::IDraw()
{
	UIBox::Draw();

	auto& dev2 = mResources->DeviceContext2;

	dev2->PushAxisAlignedClip(this->PosInfo.AbsoluteBounds, D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);

	dev2->DrawTextW(Text.c_str(), (UINT32)wcslen(Text.c_str()), *(mResources->UIConfig->SmallTextFormat->GetFormat()), this->PosInfo.AbsoluteBounds, *(mResources->UIConfig->Text->GetBrush()));

	dev2->PopAxisAlignedClip();
}*/
