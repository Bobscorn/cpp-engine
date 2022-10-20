#include "Input.h"

#include "Helpers/TransferHelper.h"

#include "Systems/Events/Events.h"
#include "Helpers/DebugHelper.h"

#include <SDL2/SDL.h>

Listener::KeyListener::KeyListener(size_t count, Input::Key * codes) : IEventListener(std::vector<Events::Event>(1, Events::KeyEvent)), KeyStates()
{
	for (size_t i = count; i-- > 0; )
	{
		KeyStates[codes[i]] = false;
	}
}

bool Listener::KeyListener::Receive(Events::IEvent * event)
{
	if (auto * kEvent = Events::ConvertEvent<Event::KeyInput>(event))
	{
		if (KeyStates.find(kEvent->KeyCode) == KeyStates.end())
			return false;

		KeyStates[kEvent->KeyCode] = kEvent->State;
		return true;
	}
	return false;
}

bool Listener::KeyListener::StateOf(Input::Key code)
{
	if (KeyStates.find(code) == KeyStates.end())
	{
		DWARNING("given invalid code");
		return false;
	}
	return KeyStates[code];
}

Listener::MouseListener::MouseListener(double x, double y, double width, double height) : IEventListener({ Events::MouseMoveEvent, Events::MouseButtonEvent, Events::WindowFocusEvent }), Mouse{ 0, 0 }//, Trigger(trigger) 
{ 
	UpdateArea(DOUBLE4(x, y, width, height)); 
}

void Listener::MouseListener::UpdateArea(DOUBLE4 area)
{
	this->Area = area;
	this->InverseDimensions.x = 1.0f / area.z;
	this->InverseDimensions.y = 1.0f / area.w;
}

bool Listener::MouseListener::Receive(Events::IEvent * event)
{
	if (auto *  mevent = Events::ConvertEvent<Event::MouseMove>(event))
	{

		return true;
	}
	return false;
}

DOUBLE2 Listener::MouseListener::DeltaAbs(CommonResources *resources)
{
	DOUBLE2 delta = DOUBLE2(0.0, 0.0);
	double dx = Mouse.x - double(*resources->CenterX);
	double dy = Mouse.y - double(*resources->CenterY);
	
	delta.x = dx;

	delta.y = dy;

	return delta;
}

DOUBLE2 Listener::MouseListener::DeltaAbsAnchor(CommonResources * resources)
{
	Anchor(resources);
	return DeltaAbs(resources);
}

void Listener::MouseListener::Anchor(CommonResources *resources)
{
	// Anchor the cursor to the target (assume target is in middle of bounding box)
	SDL_WarpMouseInWindow(resources->Window, *resources->CenterX, *resources->CenterY);
}

bool Trigger::KeyTrigger::Receive(Events::IEvent * event)
{
	if (auto * kevent = Events::ConvertEvent<Event::KeyInput>(event))
	{
		if (kevent->KeyCode == KeyCode)
		{
			KeyState = kevent->State;
			return true;
		}
		return false;
	}
	DERROR("given incorrect event");
	return false;
}

bool Trigger::MouseTrigger::Receive(Events::IEvent * event)
{
	if (auto * mevent = Events::ConvertEvent<Event::MouseButton>(event))
	{
		if (Button == mevent->Button)
		{
			State = mevent->State;
			return true;
		}
		return false;
	}
	DERROR("given incorrect event");
	return false;
}

bool Listener::Checker::Receive(Events::IEvent * event)
{
	return (HasBeenOn |= *event == Trigger.get());
}

bool Event::MouseWheel::operator==(Events::IEvent * other)
{
	if (this->Type != other->Type)
		return false;

	MouseWheel *copy = dynamic_cast<MouseWheel*>(other);
	if (!(copy))
	{
		DERROR("given incorrect event");
		return false;
	}

	return this->Scrollage == copy->Scrollage;
}

bool Event::MouseButton::operator==(Events::IEvent * other)
{
	if (this->Type != other->Type)
		return false;

	MouseButton *copy = dynamic_cast<MouseButton*>(other);
	if (!(copy))
	{
		DERROR("given incorrect event");
		return false;
	}

	return (this->Button == copy->Button && this->State == copy->State);
}

bool Event::MouseMove::operator==(Events::IEvent * other)
{
	if (this->Type != other->Type)
		return false;

	MouseMove *copy = dynamic_cast<MouseMove*>(other);
	if (!(copy))
	{
		DERROR("given incorrect event");
		return false;
	}

	return (this->_X == copy->_X && this->_Y == copy->_Y);
}

bool Event::KeyInput::operator==(Events::IEvent * other)
{
	if (this->Type != other->Type)
		return false;

	KeyInput *copy = dynamic_cast<KeyInput*>(other);
	if (!(copy))
	{
		DERROR("given incorrect event");
		return false;
	}

	return (this->KeyCode == copy->KeyCode && this->State == copy->State);
}


void Input::KeyTimeListener::Tick()
{
	for (auto& timer : KeyTimes)
	{
		timer.second.Tick();
	}
}

bool Input::KeyTimeListener::Add(Input::Key key)
{	
	auto it = KeyTimes.find(key);
	if (it != KeyTimes.end())
		return false;

	auto& tim = KeyTimes[key];
	tim.Reset();
	tim.Stop();

	return true;
}

bool Input::KeyTimeListener::Remove(Input::Key key)
{
	auto it = KeyTimes.find(key);
	if (it == KeyTimes.end())
		return false;

	KeyTimes.erase(key);

	return true;
}

bool Input::KeyTimeListener::Receive(Events::IEvent * event)
{
	if (auto *kevent = Events::ConvertEvent<Event::KeyInput>(event))
	{
		Input::Key key = kevent->KeyCode;

		auto it = KeyTimes.find(key);
		if (it != KeyTimes.end())
		{
			if (kevent->State)
				it->second.Start();
			else
				it->second.Stop();
		}

		return Events::RelevantEvent;
	}

	return Events::IrrelevantEvent;
}

bool Input::KeyPressListener::Receive(Events::IEvent * event)
{
	if (auto kv = Events::ConvertEvent<Event::KeyInput>(event))
	{
		if (kv->KeyCode == Key)
		{
			Pressed |= kv->State;
		}

		return Events::RelevantEvent;
	}
	return Events::IrrelevantEvent;
}

bool Event::MouseWheelButton::operator==(Events::IEvent * other)
{
	if (auto *e = Events::ConvertEvent<MouseWheelButton>(other))
	{
		return e->FrameID == this->FrameID && e->_X == this->_X && e->_Y == this->_Y && e->State == this->State;
	}
	return false;
}
