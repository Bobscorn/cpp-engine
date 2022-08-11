//#include "Thing.h"
#include "UIMouseHold.h"

#pragma warning(suppress:4100)
bool UI1I::Mechanic::UILeftMouseHold::OnMouseLeftDown(floaty2 & coords)
{
	timmy.Start();

	return true;
}

#pragma warning(suppress:4100)
bool UI1I::Mechanic::UILeftMouseHold::OnMouseLeftUp(floaty2 & coords)
{
	timmy.Stop();

	return true;
}

double UI1I::Mechanic::UIMouseHold::DeltaTime()
{
	timmy.Tick();
	
	double CurrentTime = timmy.TotalTime();
	
	double Delta = CurrentTime - PreviousTime;
	PreviousTime = CurrentTime;
	
	return Delta;
}
