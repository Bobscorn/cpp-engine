#pragma once

#include "EventsBase.h"

#include <type_traits>

namespace Events
{
	template<class E, typename = std::enable_if_t<std::is_base_of_v<Events::IEvent, E>>>
	E *ConvertEvent(Events::IEvent *event)
	{
		if (!event)
			return nullptr;

#ifdef _DEBUG
		return dynamic_cast<E*>(event);
#else
		if (event->Type == E::MyType)
			return (E*)event;
		else
			return nullptr;
#endif
	}
}
