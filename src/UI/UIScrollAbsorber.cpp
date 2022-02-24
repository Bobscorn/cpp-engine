//#include "Thing.h"
#include "UIScrollAbsorber.h"

#pragma warning(suppress:4100)
bool UI1I::Mechanic::UIScrollAbsorber::OnMouseScroll(floaty2 & coords, INT Scrollage)
{
	AbsorbedScroll += Scrollage;

	return true;
}

INT UI1I::Mechanic::UIScrollAbsorber::ReleaseScroll()
{
	INT Tmp = AbsorbedScroll;
	AbsorbedScroll = 0;
	return Tmp;
}
