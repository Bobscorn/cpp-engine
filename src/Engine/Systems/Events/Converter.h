#pragma once

#include "EventsBase.h"

namespace Events
{
	template<class E>
	inline E *ConvertEvent(Events::IEvent *event)
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
