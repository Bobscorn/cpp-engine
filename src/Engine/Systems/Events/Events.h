#pragma once

#include "EventsBase.h"
#include "Converter.h"
#ifdef EC_PROFILE
#include "Helpers/ProfileHelper.h"
#endif
#include "Helpers/DebugHelper.h"

#include "Math/inty.h"

#include <functional>
#include <map>
#include <unordered_map>
#include <type_traits>

#ifdef EC_PROFILE
#ifdef _DEBUG
#define EVENT_PROFILE_PUSH(x) profiler->Push(x, __FUNCTION_NAME__, __LINE__)
#define EVENT_PROFILE_POP() profiler->Pop(__FUNCTION_NAME__, __LINE__)
#else
#define EVENT_PROFILE_PUSH(x) profiler->Push(x)
#define EVENT_PROFILE_POP() profiler->Pop()
#endif
#else
#define EVENT_PROFILE_PUSH(x)
#define EVENT_PROFILE_POP()
#endif

/**
* Event Types are as follows
* Key = A Key press or release
* MouseMove = Mouse movement
* MouseScroll = Mouse Wheel Movement
* MouseButton = Mouse button press or release
* ControllerButton = Button press/release from a controller - NOT IMPLEMENTED
* ControllerAxis = Axis type event from a controller - NOT IMPLEMENTED
* 
* Resize = Window/Engine Changed Area
* ReleaseGraphics = Device lost or similar event requiring release of all graphics devices (usually followed by CreateGraphics event)
* CreateGraphics = Create all resources assuming no outstanding references remain
* 
* Random = Test event
* 
* Event Groups
* Window - Resize, ReleaseGraphics and CreateGraphics
* Input - Key, MouseMove, MouseButton, MouseScroll, ControllerButton, ControllerAxis
* 
* 
* 
*/
namespace Events
{
	struct IEvent;

	struct IEventListener;
	struct IGroupListener;


#ifdef ENUM_EVENT_TYPES

	constexpr std::array<EventGroup, 5u> EventsVec =
	{
		UnknownGroup,
		InputGroup,
		WindowGroup,
		PhysicsGroup,
		InfoGroup
	};
	
	inline constexpr EventGroup GetEventGroup(const Event& e)
	{		
		auto index = (unsigned char)e;
		if (index >= EventsVec.size())
			return UnknownGroup;

		return EventsVec[index];
	}

	inline Stringy EventName(const Event& e)
	{
		switch (e)
		{
		default:
		case UnknownEvent:
			return "UnknownEvent";
		case KeyEvent:
			return "KeyEvent";
		case MouseMoveEvent:
			return "MouseMoveEvent";
		case MouseScrollEvent:
			return "MouseScrollEvent";
		case MouseButtonEvent:
			return "MouseButtonEvent";
		case ControllerButtonEvent:
			return "ControllerButtonEvent";
		case ControllerAxisEvent:
			return "ControllerAxisEvent";
		case WindowFocusEvent:
			return "WindowFocusEvent";
		case ResizeWindowEvent:
			return "ResizeWindowEvent";
		case ReleaseGraphicsEvent:
			return "ReleaseGraphicsEvent";
		case CreateGraphicsEvent:
			return "CreateGraphicsEvent";
		case DpiChangeEvent:
			return "DpiChangeEvent";
		case BeforePhysicsEvent:
			return "BeforePhysicsEvent";
		case AfterPhysicsEvent:
			return "AfterPhysicsEvent";
		case FrameCompleteEvent:
			return "FrameCompleteEvent";
		case FontChangeEvent:
			return "FontChangeEvent";
		case MarginChangeEvent:
			return "MarginChangeEvent";
		}
	}

	inline Stringy GroupName(const EventGroup& e)
	{
		switch (e)
		{
		default:
		case UnknownGroup:
			return "UnknownGroup";
		case InputGroup:
			return "InputGroup";
		case WindowGroup:
			return "WindowGroup";
		case PhysicsGroup:
			return "PhysicsGroup";
		case InfoGroup:
			return "InfoGroup";
		}
	}
#else

	const std::unordered_map<Event, EventGroup> EventsMap = 
	{
		{ Type::EventKey,				GroupType::InputGroup },
		{ Type::EventMouseMove,			GroupType::InputGroup },
		{ Type::EventMouseScroll,		GroupType::InputGroup },
		{ Type::EventMouseButton,		GroupType::InputGroup },
		{ Type::EventControllerButton,	GroupType::InputGroup },
		{ Type::EventControllerAxis,	GroupType::InputGroup },
		{ Type::EventResizeWindow,		GroupType::WindowGroup },
		{ Type::EventReleaseGraphics,	GroupType::WindowGroup },
		{ Type::EventCreateGraphics,	GroupType::WindowGroup },
		{ Type::EventDpiChange,			GroupType::WindowGroup },
		{ Type::EventWindowFocus,		GroupType::WindowGroup },
		{ Type::EventBeforePhysics,		GroupType::PhysicsGroup },
		{ Type::EventInternalPhysics,	GroupType::PhysicsGroup },
		{ Type::EventAfterPhysics,		GroupType::PhysicsGroup },
		{ Type::EventFrameComplete,		GroupType::InfoGroup },
		{ Type::EventFontChange,		GroupType::InfoGroup },
		{ Type::EventMarginChange,		GroupType::InfoGroup }
	};

	inline EventGroup GetEventGroup(const Event& e)
	{
		auto it = EventsMap.find(e);
		if (it != EventsMap.end())
			return it->second;
		
		return GroupType::UnknownGroup;
	}

	inline Stringy EventName(const Event& e)
	{
		return e + "Event";
	}

	inline Stringy GroupName(const EventGroup& e)
	{
		return e;
	}
#endif

	const std::vector<Event> MouseEvents{ MouseMoveEvent, MouseButtonEvent, MouseScrollEvent };
	

	template<bool ... asses>
	struct gotem
	{
	};

	template<>
	struct gotem<>
	{
		constexpr static bool value = false;
	};

	template<bool ass>
	struct gotem<ass>
	{
		static constexpr bool value = ass;
	};

	template<bool ass, bool ... asses>
	struct gotem<ass, asses...>
	{
		static constexpr bool value = ass || gotem<asses...>::value;
	};

	template<class ... Ts>
	struct Goat
	{};

	template<class T, class ... Ts>
	struct Goat<T, Ts...>
	{
		static constexpr bool value = gotem<std::is_same<T, Ts>::value...>::value;
	};

	class EventManager
	{
	protected:
		// Whether the window is focused right now
		// When not focused default behaviour mutes any input based events
		bool Window_Focused = true;

		// These listeners listen to specific events
		std::unordered_map<Event, std::vector<IEventListener*>> Listeners; 

		// These listeners listen to event groups
		std::unordered_map<EventGroup, std::vector<IGroupListener*>> GroupListeners;

		// Not tested very well, supposed to be a hierachal like System in the case where perhaps the CommonResources attributes or mouse positions require tinkering for a specific set of listeners
		EventManager *m_FallTo;

		void RemoveWithoutWarning(Event &id, IEventListener *&listener);
	public:
		EventManager() : m_FallTo(nullptr) {};
		virtual ~EventManager();

		//bool Send(IEvent *event);

		template<class E>
		std::enable_if_t<Goat<E, ::Event::KeyInput, ::Event::MouseMove, ::Event::MouseWheel, ::Event::MouseButton, ::Event::MouseWheelButton, ::Event::ResizeEvent, ::Event::ResizePreEvent, ::Event::ReleaseGraphicsEvent, ::Event::CreateGraphicsEvent, ::Event::DpiChangeEvent, ::Event::WindowFocusEvent, ::Event::BeforePhysicsEvent, ::Event::InternalPhysicsEvent, ::Event::AfterPhysicsEvent, ::Event::FontSizeEvent, ::Event::MarginChangeEvent>::value, bool>
#ifdef EC_PROFILE
			Send(E *e, ProfileMcGee *profiler = nullptr);
#else
			Send(E *e);
#endif
#ifdef EC_PROFILE
		bool Send(IEvent *event, ProfileMcGee *profiler = nullptr);
#else
		bool Send(IEvent *event);
#endif

		// Note: no checks for duplicates in the Add functions
		void Add(IEventListener *listener);
		void Add(Event &id, IEventListener *listener);
		void Add(std::vector<IEventListener*> listeners);
		void Add(UINT count, IEventListener *listeners[]);
		template<std::size_t size>
		void Add(IEventListener *(&listeners)[size]);

		void Add(IGroupListener *listener);
		void Add(EventGroup &id, IGroupListener *listener);
		void Add(std::vector<IGroupListener*> listeners);
		void Add(UINT count, IGroupListener *listeners[]);
		template<std::size_t size>
		void Add(IGroupListener *(&listeners)[size]);

		void Remove(IEventListener *listener);
		void Remove(const Event & id, IEventListener *&listener);
		void Remove(std::vector<IEventListener*> listeners);
		void Remove(UINT count, IEventListener *listeners[]);
		template<std::size_t size>
		void Remove(IEventListener *(&listeners)[size]);

		void Remove(IGroupListener *listener);
		void Remove(const EventGroup & id, IGroupListener *&listener);
		void Remove(std::vector<IGroupListener*> listeners);
		void Remove(UINT count, IGroupListener *listeners[]);
		template<std::size_t size>
		void Remove(IGroupListener *(&listeners)[size]);

		inline void FallOnto(EventManager *em) { this->m_FallTo = em; };

	};

	template<class E>
	inline std::enable_if_t<Goat<E, ::Event::KeyInput, ::Event::MouseMove, ::Event::MouseWheel, ::Event::MouseButton, ::Event::MouseWheelButton, ::Event::ResizeEvent, ::Event::ResizePreEvent, ::Event::ReleaseGraphicsEvent, ::Event::CreateGraphicsEvent, ::Event::DpiChangeEvent, ::Event::WindowFocusEvent, ::Event::BeforePhysicsEvent, ::Event::InternalPhysicsEvent, ::Event::AfterPhysicsEvent, ::Event::FontSizeEvent, ::Event::MarginChangeEvent>::value, bool> 
#ifdef EC_PROFILE
		EventManager::Send(E * e, ProfileMcGee *profiler)
#else
		EventManager::Send(E * e)
#endif
	{
#ifdef EC_PROFILE
		if (profiler)
			EVENT_PROFILE_PUSH("Event: " + Events::EventName(e->Type));
#endif
		bool sent = false;

		if constexpr (std::is_same_v<E, ::Event::WindowFocusEvent>)
			Window_Focused = e->GainedFocus;

		constexpr EventGroup group = GetEventGroup(E::MyType);
		
		if constexpr (group == Events::InputGroup)
		{
			if (!Window_Focused)
				return false;
		}

#ifdef EC_PROFILE
		if (profiler)
			EVENT_PROFILE_PUSH("Event " + Events::EventName(e->Type) + " Group Listeners");
#endif
		for (auto& glisten : GroupListeners[group])
		{
			sent |= glisten->Receive(e);
		}
#ifdef EC_PROFILE
		if (profiler)
			EVENT_PROFILE_POP();
#endif
		
		// Look through Listeners
		// v
#ifdef EC_PROFILE
		if (profiler)
			EVENT_PROFILE_PUSH("EventListeners");
#endif
		auto it = Listeners.find(E::MyType);
		if (it != Listeners.end())
		{
			// Send event to all Listeners, send warning in case a listener returns false (shouldn't happen)
			// Do a check if the listener actually takes the poo (it will return true)
			// Although it shouldn't happen that a listener doesn't take an poo that it is mapped to
			// There's no guarantee
			const std::vector<IEventListener*> &ass = (*it).second;
			for (auto i = ass.size(); i-- > 0; )
			{
#ifdef EC_PROFILE
				if (profiler)
					EVENT_PROFILE_PUSH(ass[i]->GetName() + " Receive");
#endif
				bool good = ass[i]->Receive(e);
				sent |= good;
				if constexpr (group != Events::InputGroup)
				{
					if (!good)
					{
						DERROR("Event Listener found in map which doesn't take correct Event");
					}
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
					EVENT_PROFILE_PUSH(ass[i]->GetName() + " PostReceive");
#endif
				ass[i]->PostReceive(e);
#ifdef EC_PROFILE
				if (profiler)
					EVENT_PROFILE_POP();
#endif
			}
		}
		else // Not contained in Listeners, check the m_FallTo pointer
		{
			if (m_FallTo)
				sent = m_FallTo->Send(e);
		}
#ifdef EC_PROFILE
		if (profiler)
		{
			EVENT_PROFILE_POP();
			EVENT_PROFILE_POP();
		}
#endif
		return sent;
	}

	template<std::size_t size>
	inline void EventManager::Add(IEventListener *(&listeners)[size])
	{
		for (auto& listener : listeners)
		{
			Add(listener);
		}
	}

	template<std::size_t size>
	inline void EventManager::Add(IGroupListener *(&listeners)[size])
	{
		for (auto& listener : listeners)
		{
			Add(listener);
		}
	}

	template<std::size_t size>
	inline void EventManager::Remove(IEventListener *(&listeners)[size])
	{
		for (auto& listener : listeners)
		{
			Remove(listener);
		}
	}

	template<std::size_t size>
	inline void EventManager::Remove(IGroupListener *(&listeners)[size])
	{
		for (auto& listener : listeners)
		{
			Remove(listener);
		}
	}
}


namespace Event
{
	/// <summary> Sent when the window's focus gains or losses focus (WM_ACTIVATE message) </summary>
	/// <param name='GainedFocus'> is a boolean containing the new state of the window </param>
	struct WindowFocusEvent : Events::IEvent
	{
		constexpr static Events::Event MyType = Events::Event::WindowFocusEvent;
		constexpr WindowFocusEvent(bool state) : IEvent(MyType), GainedFocus(state) {};
		constexpr WindowFocusEvent(bool state, unsigned int frameid) : IEvent(MyType, frameid), GainedFocus(state) {};
		bool GainedFocus; // True = Focus gained, false = focus lost
		virtual bool operator==(Events::IEvent *other) override;
	};

	/// <summary> Exactly the same as ResizeEvent, except this Event is sent before the Standard ResizeEvent </summary>
	/// <remarks> 
	/// This is used mainly to allow for the case where something needs to reset resources *before* anything else does any resize behaviour
	/// Most notably allowing the UIConfig to send out a FontSizeChange event when a resize occurs, *before* any resizing is done
	/// allowing objects receiving FontSizeChange events to reset their font-dependant resources before than resizing
	/// </remarks>
	struct ResizePreEvent : public Events::IEvent
	{
		constexpr static Events::Event MyType = Events::Event::ResizePreEvent;
		constexpr ResizePreEvent(Vector::inty2 dims, unsigned int frameid = 0) : IEvent(MyType, frameid), Dimensions(dims) {}
		Vector::inty2 Dimensions;
		virtual bool operator==(Events::IEvent *other) override;
	};

	/// <summary> Sent when the User has finished changing the Window size (WM_EXITSIZEMOVE) </summary>
	/// <param name='Dimensions'> are the new width and height of the Window </param>
	struct ResizeEvent : public Events::IEvent
	{
		constexpr static Events::Event MyType = Events::Event::ResizeWindowEvent;
		constexpr ResizeEvent(Vector::inty2 dims) : IEvent(MyType), Dimensions(dims) {}
		constexpr ResizeEvent(Vector::inty2 dims, unsigned int frameid) : IEvent(MyType, frameid), Dimensions(dims) {}
		Vector::inty2 Dimensions;
		virtual bool operator==(Events::IEvent *other) override;
	};

	struct CreateGraphicsEvent : public Events::IEvent
	{
		constexpr static Events::Event MyType = Events::Event::CreateGraphicsEvent;
		constexpr CreateGraphicsEvent() : IEvent(MyType) {}
		constexpr CreateGraphicsEvent(unsigned int frameid) : IEvent(MyType, frameid) {}
		virtual bool operator==(Events::IEvent *other) override;
	};

	struct ReleaseGraphicsEvent : Events::IEvent
	{
		constexpr static Events::Event MyType = Events::Event::ReleaseGraphicsEvent;
		constexpr ReleaseGraphicsEvent() : Events::IEvent(MyType) {}
		constexpr ReleaseGraphicsEvent(unsigned int frameid) : Events::IEvent(MyType, frameid) {}
		virtual bool operator==(Events::IEvent *other) override;
	};

	struct DpiChangeEvent : Events::IEvent
	{
		constexpr static Events::Event MyType = Events::Event::DpiChangeEvent;
		constexpr DpiChangeEvent(float newdpi, float olddpi) : Events::IEvent(MyType), NewDpiScale(newdpi), OldDpiScale(olddpi), DpiChangeScale(newdpi / olddpi) {}
		constexpr DpiChangeEvent(float newdpi, float olddpi, unsigned int frameid) : Events::IEvent(MyType, frameid), NewDpiScale(newdpi), OldDpiScale(olddpi), DpiChangeScale(newdpi / olddpi) {}
		float NewDpiScale;
		float OldDpiScale;
		float DpiChangeScale;
		virtual bool operator==(Events::IEvent *other) override;
	};
}