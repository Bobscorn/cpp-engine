#include "UI1.h"
//#include "Thing.h"

#include "Helpers/IteratorHelper.h"
#include "Helpers/TransferHelper.h"

#include "Systems/Input/Input.h"
#include "Systems/Input/Config1.h"
#include "Drawing/Graphics2D.h"

#include <algorithm>

UI1::UIElement * UI1::UIElement::FindElement(floaty2 coords, bool trace)
{
	UIElement * e = FindOverride(coords);
	if (e)
		return e;

	if (trace)
	{
		DINFO("Tracing Find in element '" + GetName() + "' with coords (" + std::to_string(coords.x) + ", " + std::to_string(coords.y) + ")");
		for (auto& child : Iteration::reverse(Children))
		{
			if (child->IsUIEnabled())
			{
				if (child->Within(coords))
				{
					DINFO("Child '" + child->GetName() + "' was enabled and returned true for Within check");
					DINFO("Not Checking other children");
					return child;
				}
				else
				{
					DINFO("Child '" + child->GetName() + "' was enabled and returned false for Within check");
				}
			}
			else
			{
				DINFO("Child '" + child->GetName() + "' was not enabled");
			}
		}
	}
	else
	{
		for (auto& child : Iteration::reverse(Children))
		{
			if (child->IsUIEnabled() && child->Within(coords))
				return child;
		}
	}
	return nullptr;
}

bool UI1::UIElement::MouseMove(floaty2 coords)
{
	auto *child = FindElement(coords);
	
	if (child)
	{
		if (!child->MouseMove(coords))
			return this->OnMouseMove(coords);
		return true;
	}
	else 
		return this->OnMouseMove(coords);
}

void UI1::UIElement::MouseLeftClick(floaty2 coords)
{
	if (Root->GetLeftMouseDown() == nullptr)
		this->OnMouseLeftClick(coords);
	else
	{
		Root->GetLeftMouseDown()->OnMouseLeftClick(coords);
		Root->ResetLeftMouseDown();
	}
}

void UI1::UIElement::MouseRightClick(floaty2 coords)
{
	if (Root->GetRightMouseDown() == nullptr)
		this->OnMouseRightClick(coords);
	else
	{
		Root->GetRightMouseDown()->OnMouseRightClick(coords);
		Root->GetRightMouseDown();
	}
}

void UI1::UIElement::MouseWheelClick(floaty2 coords)
{
	if (Root->GetWheelDown() == nullptr)
		this->OnWheelClick(coords);
	else
	{
		Root->GetWheelDown()->OnWheelClick(coords);
		Root->GetWheelDown();
	}
}

bool UI1::UIElement::MouseScroll(floaty2 coords, INT scrollage)
{
	auto *child = FindElement(coords);

	if (child)
	{
		if (!child->MouseScroll(coords, scrollage))
			return this->OnMouseScroll(coords, scrollage);
		return true;
	}
	else
		return this->OnMouseScroll(coords, scrollage);
}

bool UI1::UIElement::MouseLeftDown(floaty2 coords)
{
	auto *child = FindElement(coords, Root->DoTrace());
	
	if (child)
	{
		if (!child->MouseLeftDown(coords))
			return Root->LMBD(this, coords);
		return true;
	}
	else
		return Root->LMBD(this, coords);
}

bool UI1::UIElement::MouseRightDown(floaty2 coords)
{
	auto *child = FindElement(coords);
	
	if (child)
	{
		if (!child->MouseRightDown(coords))
			return Root->RMBD(this, coords);
		return true;
	}
	else
		return Root->RMBD(this, coords);
}

bool UI1::UIElement::MouseWheelDown(floaty2 coords)
{
	auto *child = FindElement(coords);

	if (child)
	{
		if (!child->MouseWheelDown(coords))
			return Root->WMBD(this, coords);
		return true;
	}
	else
		return Root->WMBD(this, coords);
}

bool UI1::UIElement::MouseLeftUp(floaty2 coords)
{
	auto *child = FindElement(coords);
	
	if (child)
	{
		if (!child->MouseLeftUp(coords))
			return Root->LMBU(this, coords);
		return true;
	}
	else
	{
		return Root->LMBU(this, coords);
	}
}

bool UI1::UIElement::MouseRightUp(floaty2 coords)
{
	auto *child = FindElement(coords);

	if (child)
	{
		if (!child->MouseRightUp(coords))
			return Root->RMBU(this, coords);
		return true;
	}
	else
	{
		return Root->RMBU(this, coords);
	}
}

bool UI1::UIElement::MouseWheelUp(floaty2 coords)
{
	auto *child = FindElement(coords);

	if (child)
	{
		if (!child->MouseWheelUp(coords))
			return Root->WMBU(this, coords);
		return true;
	}
	else
	{
		return Root->WMBU(this, coords);
	}
}

bool UI1::UIElement::MouseHover(floaty2 coords)
{
	auto *child = FindElement(coords);

	if (child)
	{
		if (!child->MouseHover(coords))
			return this->OnHoveringThisFrame(*mResources->UpdateID, coords);
		return true;
	}
	else
		return this->OnHoveringThisFrame(*mResources->UpdateID, coords);
}

bool UI1::RootElement::Receive(Events::IEvent * event)
{
	(void)event;
	return Events::IrrelevantEvent;
}

bool UI1::RootElement::Receive(Event::WindowFocusEvent * winfocus)
{
	LeftMouseDownEl = nullptr;
	RightMouseDownEl = nullptr;
	if (!winfocus->GainedFocus && this->Focus)
	{
		if (!Focus->OnFocusLost(UIElement::FocusLoss::FL_WINDOWLOSS, nullptr))
			Focus = nullptr;
	}
	return Events::RelevantEvent;
}

bool UI1::RootElement::Receive(Event::ResizeEvent * resize)
{
	(void)resize;
	ComputePositions();
	return Events::RelevantEvent;
}

bool UI1::RootElement::Receive(Event::MouseWheel * mwheel)
{
	floaty2 localCoords = floaty2::TLtoGL({ float(mwheel->_X), float(mwheel->_Y) }, { *mResources->HalfWindowWidth, *mResources->HalfWindowHeight });

	MouseScroll(localCoords, mwheel->Scrollage);

	return Events::RelevantEvent;
}

bool UI1::RootElement::Receive(Event::MouseWheelButton * event)
{
	floaty2 globalCoords = floaty2::TLtoGL({ float(event->_X), float(event->_Y) }, { *mResources->HalfWindowWidth, *mResources->HalfWindowHeight });
	if (event->State)
	{
		WheelDownEl = nullptr;
		MouseWheelDown(globalCoords);
	}
	else
	{
		MouseWheelUp(globalCoords);
	}

	return Events::RelevantEvent;
}

bool UI1::RootElement::Receive(Event::MouseButton * mb)
{
	floaty2 localCoords = floaty2::TLtoGL({ float(mb->_X), float(mb->_Y) }, { *mResources->HalfWindowWidth, *mResources->HalfWindowHeight });

	//DINFO("Mouse Coords: (" + std::to_string(localCoords.x) + ", " + std::to_string(localCoords.y) + ")");

	if (mb->Button == LMB)
	{
		if (mb->State)
		{
			LeftMouseDownEl = nullptr;
			MouseLeftDown(localCoords);
		}
		else
		{
			MouseLeftUp(localCoords);
		}
	}
	else if (mb->Button == RMB)
	{
		if (mb->State)
		{
			RightMouseDownEl = nullptr;
			MouseRightDown(localCoords);
		}
		else
		{
			MouseRightUp(localCoords);
		}
	}

	return Events::RelevantEvent;
}

bool UI1::RootElement::Receive(Event::MouseMove * mm)
{
	if (DoMouseMovement)
	{
		floaty2 localCoords = floaty2::TLtoGL({ float(mm->_X), float(mm->_Y) }, { *mResources->HalfWindowWidth, *mResources->HalfWindowHeight });

		MouseMove(localCoords);	
	}
	return Events::RelevantEvent;
}

bool UI1::RootElement::Receive(Event::KeyInput * key)
{
	(void)key;
	return false;
}

void UI1::RootElement::Superseded()
{
	LeftMouseDownEl = nullptr;
	RightMouseDownEl = nullptr;
	if (Focus && !Focus->OnFocusLost(UIElement::FocusLoss::FL_WINDOWLOSS, nullptr))
		Focus = nullptr;
}

void UI1::RootElement::Reinstated()
{
}

void UI1::RootElement::ChildAdded(UIElement * el)
{
	el->RootPosition();
}

void UI1::RootElement::ChildrenAdded(std::vector<UIElement*> els)
{
	for (auto& el : els)
		el->RootPosition();
}

void UI1::UIElement::BeforeDraw()
{
	IBeforeDraw();

	for (auto& child : Children)
		child->BeforeDraw();
}

void UI1::UIElement::Draw()
{
	IDraw();

	for (auto& child : Children)
		if (child->IsUIEnabled())
			child->Draw();
}

void UI1::UIElement::AfterDraw()
{
	IAfterDraw();

	for (auto& child : Children)
		child->AfterDraw();
}

Debug::DebugReturn UI1::UIElement::AddChildBottom(UIElement * e)
{
	return AddChild(GetBottomDepth(), e);
}

Debug::DebugReturn UI1::UIElement::AddChild(UINT depth, UIElement * e)
{
	if (!e->GetResources())
		e->SetResources(mResources);

	e->Parent = this;
	e->Root = this->Root;
	e->Depth = depth;
	Children.push_back(e);
	std::stable_sort(Children.begin(), Children.end(), UIElementLess());

	auto d = e->Initialize();

	this->ChildAdded(e);

	return d;
}

bool UI1::UIElement::BeingClicked() const
{
	return (Root && Root->GetLeftMouseDown() == this);
}

Debug::DebugReturn UI1::UIElement::AddChildrenBottom(std::vector<UIElement*> els)
{
	return AddChildren<UIElement>(GetBottomDepth(), els);
}

Debug::DebugReturn UI1::UIElement::AddChildrenTop(std::vector<UIElement*> els)
{
	return AddChildren<UIElement>(GetTopDepth(), els);
}

Debug::DebugReturn UI1::UIElement::AddChildTop(UIElement * e)
{
	return AddChild(GetTopDepth(), e);
}

UI1::UIElement * UI1::RootElement::FindOverride(floaty2 coords)
{
	for (auto& child : Iteration::reverse(FloatElements))
	{
		if (child->IsUIEnabled() && child->Within(coords))
			return child;
	}
	return nullptr;
}

void UI1::RootElement::BeforeDraw()
{
	// Send Hover Event
	if (DoHover && *mResources->WindowFocused)
	{
		floaty2 MousePos = floaty2::TLtoGL({ float(mResources->MousePosition->x), float(mResources->MousePosition->y) }, { *mResources->HalfWindowWidth, *mResources->HalfWindowHeight });
		if (MousePos.x > -*mResources->HalfWindowWidth && MousePos.y > -*mResources->HalfWindowHeight && MousePos.x < *mResources->HalfWindowWidth && MousePos.y < *mResources->HalfWindowHeight)
		{
			MouseHover(MousePos);
		}
	}
	UIElement::BeforeDraw();
}

void UI1::RootElement::Draw()
{
	UIElement::Draw();

	for (auto& schild : FloatElements)
	{
		if (schild->IsUIEnabled())
			schild->Draw();
	}

	if (DebugDraw)
	{
		for (auto& child : Children)
		{
			child->DebugDraw();
		}
		for (auto& fchild : FloatElements)
		{
			fchild->DebugDraw();
		}
	}
	mResources->Ren2->SetTransform(Matrixy2x3::Identity());
}

UI1::UIFloatElement::~UIFloatElement() 
{ 
	if (Root) 
		Root->RemoveFloatChild(this); 
};

void UI1::UIElement::IDebugDraw()
{
	mResources->Ren2->SetTransform(this->GetFullMatrix());
	mResources->Ren2->DrawLine({ -3.f, 0.f }, { +3.f, 0.f }, mResources->UIConfig->GetBrush("DebugLine"));
	mResources->Ren2->DrawLine({ 0.f, -3.f }, { 0.f, +3.f }, mResources->UIConfig->GetBrush("DebugLine"));
}