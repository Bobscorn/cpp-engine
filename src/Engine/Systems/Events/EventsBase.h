#pragma once

#include "Helpers/DebugHelper.h"

#include <vector>
#include <array>
#include <functional>

namespace Events
{
#define ENUM_EVENT_TYPES
#ifdef ENUM_EVENT_TYPES
	enum GroupType : int8_t
	{
		UnknownGroup = 0,
		InputGroup = 1,
		WindowGroup = 2,
		PhysicsGroup = 3,
		InfoGroup = 4, // Essentially the InfoGroup is just miscelaneous events that I don't want in the other groups because of Group Listeners
	};

#define xInputGroup(x)		(x << 8) | GroupType::InputGroup
#define xWindowGroup(x)		(x << 8) | GroupType::WindowGroup
#define xPhysicsGroup(x)	(x << 8) | GroupType::PhysicsGroup
#define xInfoGroup(x)		(x << 8) | GroupType::InfoGroup

	enum Type : int32_t
	{
		UnknownEvent = 0,
		KeyEvent = xInputGroup(1),
		MouseMoveEvent = xInputGroup(2),
		MouseScrollEvent = xInputGroup(3),
		MouseButtonEvent = xInputGroup(4),
		MouseWheelButtonEvent = xInputGroup(5),
		ControllerButtonEvent = xInputGroup(6),
		ControllerAxisEvent = xInputGroup(7),

		ResizeWindowEvent = xWindowGroup(1),
		ResizePreEvent = xWindowGroup(2),
		ReleaseGraphicsEvent = xWindowGroup(3),
		CreateGraphicsEvent = xWindowGroup(4),
		DpiChangeEvent = xWindowGroup(5),
		WindowFocusEvent = xWindowGroup(6),

		BeforePhysicsEvent = xPhysicsGroup(1),
		InternalPhysicsEvent = xPhysicsGroup(2),
		AfterPhysicsEvent = xPhysicsGroup(3),

		FrameCompleteEvent = xInfoGroup(1),
		FontChangeEvent = xInfoGroup(2),
		MarginChangeEvent = xInfoGroup(3),
	};

#undef xInputGroup
#undef xWindowGroup
#undef xPhysicsGroup
#undef xInfoGroup

	typedef Type Event;
	typedef GroupType EventGroup;

#else
	typedef Stringy Event;
	typedef Stringy EventGroup;

	namespace Type
	{
		constexpr Event UnknownEvent = "";
		constexpr Event KeyEvent = "Key";
		constexpr Event MouseMoveEvent = "MouseMove";
		constexpr Event MouseScrollEvent = "MouseScrol";
		constexpr Event MouseButtonEvent = "MouseButton";
		constexpr Event ControllerButtonEvent = "ControllerButton";
		constexpr Event ControllerAxisEvent = "ControllerAxis";

		constexpr Event ResizeWindowEvent = "Resize";
		constexpr Event ResizePreEvent = "ResizePre";
		constexpr Event ReleaseGraphicsEvent = "ReleaseGraphics";
		constexpr Event CreateGraphicsEvent = "CreateGraphics";
		constexpr Event DpiChangeEvent = "DpiChange";
		constexpr Event WindowFocusEvent = "WindowFocus";

		constexpr Event BeforePhysicsEvent = "BeforePhysics";
		constexpr Event InternalPhysicsEvent = "InteralPhysics";
		constexpr Event AfterPhysicsEvent = "AfterPhysics";

		constexpr Event FrameCompleteEvent = "FrameComplete";
		constexpr Event FontChangeEvent = "FontChange";
		constexpr Event MarginChangeEvent = "MarginChange";
	}

	namespace GroupType
	{
		constexpr EventGroup UnknownGroup = "UnknownGroup";
		constexpr EventGroup WindowGroup = "WindowGroup";
		constexpr EventGroup InputGroup = "InputGroup";
		constexpr EventGroup PhysicsGroup = "PhysicsGroup";
		constexpr EventGroup InfoGroup = "InfoGroup";
	}

#endif

	constexpr bool RelevantEvent = true; // Return values for Receive functions
	constexpr bool IrrelevantEvent = false;

	struct IEvent
	{
		constexpr IEvent(Events::Event ass = Events::UnknownEvent, unsigned int FrameID = 0u) : FrameID(FrameID), Type(ass) {}
		Events::Event Type;
		unsigned int FrameID;
		virtual bool operator==(IEvent *other) = 0;
		inline virtual bool operator!=(IEvent *other) { return !(this->operator==(other)); };
		virtual ~IEvent();
	};


	class EventManager;

#define AVOID_DYNAMIC_CAST_EVENTS

#ifdef AVOID_DYNAMIC_CAST_EVENTS
}

namespace Event
{
	struct FontSizeEvent;
	struct MarginChangeEvent;
	struct KeyInput;
	struct MouseMove;
	struct MouseButton;
	struct MouseWheel;
	struct MouseWheelButton;
	struct BeforePhysicsEvent;
	struct InternalPhysicsEvent;
	struct AfterPhysicsEvent;
	struct WindowFocusEvent;
	struct ResizePreEvent;
	struct ResizeEvent;
	struct CreateGraphicsEvent;
	struct ReleaseGraphicsEvent;
	struct DpiChangeEvent;
}

namespace Events
{
#endif


	/// <summary> Receives Event from an EventManager, Removes itself from Manager when destroyed </summary>
	/// <param name="ListeningTo"> Pointer to the EventManager that this object is listening to </param>
	struct IEventListener
	{
		IEventListener(std::vector<Event> events) : ListeningTo(nullptr), Events(events) {};
		virtual ~IEventListener(); // Removes itself from EventManager (stored in ListeningTo) if not nullptr
		EventManager *ListeningTo;
		const std::vector<Event> Events;

		inline virtual void Added(EventManager * to) { (void)to; };
		inline virtual void Removed(EventManager * from) { (void)from; };
		inline virtual Stringy GetName() const { return "Unnamed Event Listener"; }
		virtual Stringy GetDetails() const;
#ifdef AVOID_DYNAMIC_CAST_EVENTS
		virtual bool Receive(::Event::KeyInput * event);
		virtual bool Receive(::Event::MouseMove * event);
		virtual bool Receive(::Event::MouseWheel * event);
		virtual bool Receive(::Event::MouseButton * event);
		virtual bool Receive(::Event::MouseWheelButton * event);
		//virtual bool Receive(ControllerButton * event);
		//virtual bool Receive(ControllerAxis * event);
		virtual bool Receive(::Event::ResizeEvent * event);
		virtual bool Receive(::Event::ResizePreEvent * event);
		virtual bool Receive(::Event::ReleaseGraphicsEvent * event);
		virtual bool Receive(::Event::CreateGraphicsEvent * event);
		virtual bool Receive(::Event::DpiChangeEvent * event);
		virtual bool Receive(::Event::WindowFocusEvent * event);
		virtual bool Receive(::Event::BeforePhysicsEvent * event);
		virtual bool Receive(::Event::InternalPhysicsEvent * event);
		virtual bool Receive(::Event::AfterPhysicsEvent * event);
		virtual bool Receive(::Event::FontSizeEvent * event);
		virtual bool Receive(::Event::MarginChangeEvent * event);
		virtual void PostReceive(::Event::KeyInput * event);
		virtual void PostReceive(::Event::MouseMove * event);
		virtual void PostReceive(::Event::MouseWheel * event);
		virtual void PostReceive(::Event::MouseButton * event);
		//virtual void PostReceive(ControllerButton * event);
		//virtual void PostReceive(ControllerAxis * event);
		virtual void PostReceive(::Event::ResizeEvent * event);
		virtual void PostReceive(::Event::ResizePreEvent * event);
		virtual void PostReceive(::Event::ReleaseGraphicsEvent * event);
		virtual void PostReceive(::Event::CreateGraphicsEvent * event);
		virtual void PostReceive(::Event::DpiChangeEvent * event);
		virtual void PostReceive(::Event::WindowFocusEvent * event);
		virtual void PostReceive(::Event::BeforePhysicsEvent * event);
		virtual void PostReceive(::Event::InternalPhysicsEvent * event);
		virtual void PostReceive(::Event::AfterPhysicsEvent * event);
		virtual void PostReceive(::Event::FontSizeEvent * event);
		virtual void PostReceive(::Event::MarginChangeEvent * event);
#endif
		virtual bool Receive(IEvent *event) = 0;
		virtual void PostReceive(IEvent *event) { (void)event; }
	};

	template<Event... args>
	constexpr std::array<Event, sizeof...(args)> nukem_dukem{ args... };

	template<Event... args>
	struct IEventListenerT : public IEventListener
	{
		IEventListenerT() : IEventListener(std::vector<Event>(nukem_dukem<args...>.begin(), nukem_dukem<args...>.end())) {};
		virtual ~IEventListenerT() {}
	};

	struct IInputListener : IEventListenerT<Event::MouseButtonEvent, Event::MouseMoveEvent, Event::MouseScrollEvent, Event::MouseWheelButtonEvent, Event::KeyEvent, Event::ControllerAxisEvent, Event::ControllerButtonEvent>
	{
		virtual ~IInputListener() {}
	};

	struct ILambdaListener : IEventListener
	{
		ILambdaListener(std::vector<Event> events, std::function<bool(IEvent *event)> func) : IEventListener(events), ReceiveFunc(std::move(func)) {};

		inline bool Receive(IEvent *event) override { return ReceiveFunc(event); }

		virtual ~ILambdaListener() {};
	protected:
		std::function<bool(IEvent *event)> ReceiveFunc;
	};

	struct IGroupListener
	{
		IGroupListener(EventGroup group) : ListeningTo(nullptr), Group(group) {};
		EventManager *ListeningTo;
		const EventGroup Group;

#pragma warning(suppress:4100)
		inline virtual void Added(EventManager * to) {};
#pragma warning(suppress:4100)
		inline virtual void Removed(EventManager * from) {};
		virtual bool Receive(IEvent *event) = 0;
		virtual ~IGroupListener();
	};

	template<EventGroup group>
	struct IGroupListenerT : IGroupListener
	{
		IGroupListenerT() : IGroupListener(group) {}
	};
}

namespace Listener
{
	struct IResizeListener : public Events::IEventListener
	{
		IResizeListener() : IEventListener({ Events::Type::ResizeWindowEvent }) {};
		virtual bool Receive(Events::IEvent *event) override = 0;
		virtual ~IResizeListener() {};
	};

	struct IGEventListener : public Events::IGroupListener
	{
		IGEventListener() : IGroupListener(Events::GroupType::WindowGroup) {};
		virtual bool Receive(Events::IEvent *event) override = 0;
		virtual ~IGEventListener() {};
	};
}