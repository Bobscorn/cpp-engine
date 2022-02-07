#include "EventsBase.h"

#include "Helpers/DebugHelper.h"

#include "Events.h"
#include "Systems/Input/Input.h"
#include "Systems/Input/Config1.h"
#include "PhysicsEvent.h"

Stringy Events::IEventListener::GetDetails() const
{ 
	Stringy ListensTo = (Events.size() ? "" : "Nothing, ");
	for (auto& e : Events)
		ListensTo += EventName(e) + ", ";
	ListensTo.pop_back();
	ListensTo.pop_back();
	return "This '" + GetName() + "' listens to " + ListensTo;
}

bool Events::IEventListener::Receive(::Event::KeyInput * event)
{
	return this->Receive((IEvent*)event);
}

bool Events::IEventListener::Receive(::Event::MouseMove * event)
{
	return this->Receive((IEvent*)event);
}

bool Events::IEventListener::Receive(::Event::MouseWheel * event)
{
	return this->Receive((IEvent*)event);
}

bool Events::IEventListener::Receive(::Event::MouseButton * event)
{
	return this->Receive((IEvent*)event);
}

bool Events::IEventListener::Receive(::Event::MouseWheelButton * event)
{
	return this->Receive((IEvent*)event);
}

bool Events::IEventListener::Receive(::Event::ResizeEvent * event)
{
	return this->Receive((IEvent*)event);
}

bool Events::IEventListener::Receive(::Event::ResizePreEvent * event)
{
	return this->Receive((IEvent*)event);
}

bool Events::IEventListener::Receive(::Event::ReleaseGraphicsEvent * event)
{
	return this->Receive((IEvent*)event);
}

bool Events::IEventListener::Receive(::Event::CreateGraphicsEvent * event)
{
	return this->Receive((IEvent*)event);
}

bool Events::IEventListener::Receive(::Event::DpiChangeEvent * event)
{
	return this->Receive((IEvent*)event);
}

bool Events::IEventListener::Receive(::Event::WindowFocusEvent * event)
{
	return this->Receive((IEvent*)event);
}

bool Events::IEventListener::Receive(::Event::BeforePhysicsEvent * event)
{
	return this->Receive((IEvent*)event);
}

bool Events::IEventListener::Receive(::Event::InternalPhysicsEvent * event)
{
	return this->Receive((IEvent*)event);
}

bool Events::IEventListener::Receive(::Event::AfterPhysicsEvent * event)
{
	return this->Receive((IEvent*)event);
}

bool Events::IEventListener::Receive(::Event::FontSizeEvent * event)
{
	return this->Receive((IEvent*)event);
}

bool Events::IEventListener::Receive(::Event::MarginChangeEvent * event)
{
	return this->Receive((IEvent*)event);
}

void Events::IEventListener::PostReceive(::Event::KeyInput * event)
{
	this->PostReceive((IEvent*)event);
}

void Events::IEventListener::PostReceive(::Event::MouseMove * event)
{
	this->PostReceive((IEvent*)event);
}

void Events::IEventListener::PostReceive(::Event::MouseWheel * event)
{
	this->PostReceive((IEvent*)event);
}

void Events::IEventListener::PostReceive(::Event::MouseButton * event)
{
	this->PostReceive((IEvent*)event);
}

void Events::IEventListener::PostReceive(::Event::ResizeEvent * event)
{
	this->PostReceive((IEvent*)event);
}

void Events::IEventListener::PostReceive(::Event::ResizePreEvent * event)
{
	this->PostReceive((IEvent*)event);
}

void Events::IEventListener::PostReceive(::Event::ReleaseGraphicsEvent * event)
{
	this->PostReceive((IEvent*)event);
}

void Events::IEventListener::PostReceive(::Event::CreateGraphicsEvent * event)
{
	this->PostReceive((IEvent*)event);
}

void Events::IEventListener::PostReceive(::Event::DpiChangeEvent * event)
{
	this->PostReceive((IEvent*)event);
}

void Events::IEventListener::PostReceive(::Event::WindowFocusEvent * event)
{
	this->PostReceive((IEvent*)event);
}

void Events::IEventListener::PostReceive(::Event::BeforePhysicsEvent * event)
{
	this->PostReceive((IEvent*)event);
}

void Events::IEventListener::PostReceive(::Event::InternalPhysicsEvent * event)
{
	this->PostReceive((IEvent*)event);
}

void Events::IEventListener::PostReceive(::Event::AfterPhysicsEvent * event)
{
	this->PostReceive((IEvent*)event);
}

void Events::IEventListener::PostReceive(::Event::FontSizeEvent * event)
{
	this->PostReceive((IEvent*)event);
}

void Events::IEventListener::PostReceive(::Event::MarginChangeEvent * event)
{
	this->PostReceive((IEvent*)event);
}
