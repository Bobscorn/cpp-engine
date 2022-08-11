#include "Events.h"

void Events::EventManager::RemoveWithoutWarning(Events::Event &id, IEventListener *& listener)
{
	if (Listeners.find(id) != Listeners.end())
	{
		std::vector<IEventListener*> &ass = Listeners[id];
		for (auto i = ass.size(); i-- > 0; )
		{
			if (ass[i] == listener)
				ass.erase(ass.begin() + i);
		}
	}

	if (m_FallTo)
		m_FallTo->Remove(id, listener);
}

bool Events::EventManager::Send(IEvent *event
#ifdef EC_PROFILE
	, ProfileMcGee *profiler
#endif
)
{
#ifdef EC_PROFILE
	if (profiler)
		EVENT_PROFILE_PUSH("Event: " + Events::EventName(event->Type), __FUNCTION_NAME__, __LINE__);
#endif
	bool sent = false;
		
	EventGroup group = GetEventGroup(event->Type);
	
	for (auto& glisten : GroupListeners[group])
	{
		sent |= glisten->Receive(event);
	}


	// Look through Listeners
	// v
	if (Listeners.find(event->Type) != Listeners.end())
	{
		// Send event to all Listeners, send warning in case a listener returns false (shouldn't happen)
		// Do a check if the listener actually takes the poo (it will return true)
		// Although it shouldn't happen that a listener doesn't take an poo that it is mapped to
		// There's no guarantee
		Events::Event &id = event->Type;
		const std::vector<IEventListener*> &ass = Listeners[id];
		for (auto i = ass.size(); i-- > 0; )
		{
#ifdef EC_PROFILE
			if (profiler)
				EVENT_PROFILE_PUSH(ass[i]->GetName() + " Receive", __FUNCTION_NAME__, __LINE__);
#endif 
			bool good = ass[i]->Receive(event);
			sent |= good;
			if (!good)
			{
				DERROR("Event Listener found in map which doesn't take correct Event");
			}
#ifdef EC_PROFILE
			if (profiler)
				EVENT_PROFILE_POP();
#endif
		}
		for (auto i = ass.size(); i-- > 0; )
		{
#ifdef EC_PROFILE
			if (profiler)
				EVENT_PROFILE_PUSH(ass[i]->GetName() + " PostReceive", __FUNCTION_NAME__, __LINE__);
#endif
			ass[i]->PostReceive(event);
#ifdef EC_PROFILE
			if (profiler)
				EVENT_PROFILE_POP();
#endif
		}
	}
	else // Not contained in Listeners, check the m_FallTo pointer, so far never used, although I'll probably forget to change this
	{
		if (m_FallTo)
			sent = m_FallTo->Send(event);
	}
#ifdef EC_PROFILE
	if (profiler)
		EVENT_PROFILE_POP();
#endif
	return sent;
}

void Events::EventManager::Add(IEventListener * listener)
{
	if (listener->Events.size())
		for (auto& id : listener->Events)
		{
			Listeners[id].push_back(listener);
		}
	else
		DERROR("IEventListener with no ListenedEvents");

	// Check if the listener was mapped in a different EventManager (remove if it is)
	if (listener->ListeningTo != this && listener->ListeningTo != nullptr)
		listener->ListeningTo->Remove(listener);

	// Make sure this listener knows that this manager owns it >:3
	listener->ListeningTo = this;
	listener->Added(this);
}

void Events::EventManager::Add(Events::Event & id, IEventListener * listener)
{
	if (id != Events::UnknownEvent)
		Listeners[id].push_back(listener);
	else
		DWARNING("given empty id");

	// Check if the listener was mapped in a different EventManager (remove if it is)
	if (listener->ListeningTo != this && listener->ListeningTo != nullptr)
		listener->ListeningTo->Remove(listener);

	// Make sure this listener knows that this manager owns it >:3
	listener->ListeningTo = this;
	listener->Added(this);
}

void Events::EventManager::Add(std::vector<IEventListener*> listeners)
{
	for (auto& listener : listeners)
	{
		Add(listener);
	}
}

void Events::EventManager::Add(UINT count, IEventListener * listeners[])
{
	for (UINT i = count; i-- > 0; )
	{
		Add(listeners[i]);
	}
}

void Events::EventManager::Add(IGroupListener * listener)
{
	GroupListeners[listener->Group].push_back(listener);
	if (listener->Group == Events::UnknownGroup)
		DERROR("given empty listener");

	// Check if the listener was mapped in a different EventManager (remove if it is)
	if (listener->ListeningTo != this && listener->ListeningTo != nullptr)
		listener->ListeningTo->Remove(listener);

	// Make sure this listener knows that this manager owns it >:3
	listener->ListeningTo = this;
	listener->Added(this);
}

void Events::EventManager::Add(Events::EventGroup & id, IGroupListener * listener)
{
	if (id != Events::UnknownGroup)
		GroupListeners[id].push_back(listener);
	else
		DWARNING("given empty id");

	// Check if the listener was mapped in a different EventManager (remove if it is)
	if (listener->ListeningTo != this && listener->ListeningTo != nullptr)
		listener->ListeningTo->Remove(listener);

	// Make sure this listener knows that this manager owns it >:3
	listener->ListeningTo = this;
	listener->Added(this);
}

void Events::EventManager::Add(std::vector<IGroupListener*> listeners)
{
	for (auto& listener : listeners)
	{
		Add(listener);
	}
}

void Events::EventManager::Add(UINT count, IGroupListener * listeners[])
{
	for (UINT i = count; i-- > 0; )
	{
		Add(listeners[i]);
	}
}

void Events::EventManager::Remove(IEventListener * listener)
{
	if (!listener)
	{
		DWARNING("given null listener");
		return;
	}
	const std::vector<Events::Event> &ass = listener->Events; // Might return with a blank vector
	if (!ass.empty()) // if the listener can supply a list of EventIDs to find
	{
		for (auto& eid : ass)
		{
			Remove(eid, listener);
		}
	}
	else // or just search the map
	{
		for (auto& mapping : Listeners)
		{
			Events::Event gotem = mapping.first;
			RemoveWithoutWarning(gotem, listener);
		}
	}
	listener->ListeningTo = nullptr;
	// and of course remove from the FallTo EventManager
	if (m_FallTo)
		m_FallTo->Remove(listener);
	listener->Removed(this);
}

void Events::EventManager::Remove(const Events::Event & id, IEventListener *& listener)
{
	if (Listeners.find(id) == Listeners.end())
		DWARNING("could not find event listener by name of " + EventName(id));
	
	std::vector<IEventListener*> &ass = Listeners[id];
	for (auto i = ass.size(); i-- > 0; )
	{
		if (ass[i] == listener)
			ass.erase(ass.begin() + i);
	}
	
	if (m_FallTo)
		m_FallTo->Remove(id, listener);
	listener->Removed(this);
}

void Events::EventManager::Remove(std::vector<IEventListener*> listeners)
{
	for (auto& listener : listeners)
	{
		Remove(listener);
	}
}

void Events::EventManager::Remove(UINT count, IEventListener * listeners[])
{
	for (UINT i = count; i-- > 0; )
	{
		Remove(listeners[i]);
	}
}

void Events::EventManager::Remove(IGroupListener * listener)
{
	if (listener == nullptr)
	{
		DWARNING("given null listener");
		return;
	}
	const Events::EventGroup &id = listener->Group;
	if (id == Events::UnknownGroup)
		DWARNING("given listener that returned empty Group type");
	if (GroupListeners.find(id) == GroupListeners.end())
		DWARNING("could not find group listener with id of: " + GroupName(id));
	else
	{
		std::vector<IGroupListener*> &ass = GroupListeners[id];
		for (auto i = ass.size(); i-- > 0; )
		{
			if (ass[i] == listener)
				ass.erase(ass.begin() + i);
		}
	}
	listener->ListeningTo = nullptr;

	if (m_FallTo)
		m_FallTo->Remove(id, listener);
	listener->Removed(this);
}

void Events::EventManager::Remove(const Events::EventGroup & id, IGroupListener *& listener)
{
	if (GroupListeners.find(id) == GroupListeners.end())
		DWARNING("Attempted Remove of EventID with Type: " + GroupName(id) + " where EventID could not be found");
	else
	{
		std::vector<IGroupListener*> &ass = GroupListeners[id];
		for (auto i = ass.size(); i-- > 0; )
		{
			if (ass[i] == listener)
				ass.erase(ass.begin() + i);
		}
	}

	if (m_FallTo)
		m_FallTo->Remove(id, listener);
	listener->Removed(this);
}

void Events::EventManager::Remove(std::vector<IGroupListener*> listeners)
{
	for (auto& listener : listeners)
	{
		Remove(listener);
	}
}

void Events::EventManager::Remove(UINT count, IGroupListener * listeners[])
{
	for (UINT i = count; i-- > 0; )
	{
		Remove(listeners[i]);
	}
}

Events::EventManager::~EventManager()
{
	// Remove all listeners
	for (auto& listens : Listeners)
	{
		for (auto& listen : listens.second)
		{
			Remove(listen);
		}
	}

	for (auto& glistens : GroupListeners)
	{
		for (auto& glisten : glistens.second)
		{
			Remove(glisten);
		}
	}
}

Events::IEventListener::~IEventListener()
{
	if (ListeningTo)
	{
		IEventListener *ass = this;
		ListeningTo->Remove(ass);
	}
}

Events::IEvent::~IEvent()
{
}

bool Event::ResizeEvent::operator==(Events::IEvent * other)
{
	auto *ass = Events::ConvertEvent<ResizeEvent>(other);
	return (ass && ass->Dimensions == ass->Dimensions && ass->FrameID == this->FrameID);
}

bool Event::CreateGraphicsEvent::operator==(Events::IEvent * other)
{
	auto *ass = Events::ConvertEvent<CreateGraphicsEvent>(other);
	return (ass && ass->FrameID == this->FrameID);
}

Events::IGroupListener::~IGroupListener()
{
	if (ListeningTo)
	{
		IGroupListener *ass = this;
		ListeningTo->Remove(ass);
	}
}

bool Event::ReleaseGraphicsEvent::operator==(Events::IEvent * other)
{
	auto *ass = Events::ConvertEvent<ReleaseGraphicsEvent>(other);
	return (ass && ass->FrameID == other->FrameID);
}

bool Event::WindowFocusEvent::operator==(Events::IEvent * other)
{
	auto *ass = Events::ConvertEvent<WindowFocusEvent>(other);
	return (ass && ass->FrameID == other->FrameID && ass->GainedFocus == ass->GainedFocus);
}

bool Event::DpiChangeEvent::operator==(Events::IEvent * other)
{	
	DpiChangeEvent *copy = Events::ConvertEvent<DpiChangeEvent>(other);
	return (copy && copy->FrameID == this->FrameID && this->NewDpiScale == copy->NewDpiScale && this->OldDpiScale == copy->OldDpiScale);
}

bool Event::ResizePreEvent::operator==(Events::IEvent * other)
{
	auto *ass = Events::ConvertEvent<ResizePreEvent>(other);
	return (ass && Dimensions == ass->Dimensions && ass->FrameID == FrameID);
}
