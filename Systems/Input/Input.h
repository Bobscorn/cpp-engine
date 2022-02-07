#pragma once

#include "Helpers/VectorHelper.h"

#include "Systems/Events/EventsBase.h"
#include "Systems/Timer/Timer.h"

#include <vector>
#include <unordered_map>

struct CommonResources;

enum MouseButton
{
	LMB = 0,
	RMB = 1,
	MMB = 2,
	WHEEL = 3,
};

namespace Input
{
	typedef unsigned int Key;
}

namespace Trigger
{
	struct TriggerReceiver;
	struct SingleTriggerSender;
	struct MultiTriggerSender;

	// Super simple event system, basically just a polymorphic message receiver thats not hooked up to an EventManager
	struct TriggerReceiver
	{
		virtual void Triggered() = 0;
	};

	// Super simple event system, basically polymorphic way of sending messages to specific objects without an EventManager
	struct SingleTriggerSender
	{
		SingleTriggerSender(TriggerReceiver *receiver = nullptr) : Receiver(receiver) {};
		TriggerReceiver *Receiver;
		inline void Trigger() {
			if (Receiver)
				Receiver->Triggered();
		};
	};

	// A Trigger sender but with multiple senders, this class could easily not exist (nothing relies on the polymorphic behaviour of this class)
	// But it could save multiple lines of code
	struct MultiTriggerSender
	{
		MultiTriggerSender(std::vector<TriggerReceiver*> receivers = std::vector<TriggerReceiver*>()) : Receivers(receivers) {};
		std::vector<TriggerReceiver*> Receivers;
		inline void Trigger() {
			for (auto& ass : Receivers)
			{
				if (ass)
					ass->Triggered();
			}
		};
	};

	// A basic event listener that stores whether its trigger has been pressed
	struct ITriggerListener : public Events::IGroupListener
	{
		ITriggerListener(Events::EventGroup group) : IGroupListener(group) {};
		virtual bool Triggered() = 0;
		virtual ~ITriggerListener() {};
	};

	// Trigger listener that stores whether a key is pressed
	struct KeyTrigger : public ITriggerListener
	{
		KeyTrigger(Input::Key code) : ITriggerListener(Events::InputGroup), KeyCode(code), KeyState(false) {};
		Input::Key KeyCode;
		bool KeyState;
		inline bool Triggered() override { return KeyState; };
		bool Receive(Events::IEvent *event) override;
	};

	// Trigger listener that stores whether a mouse button is pressed
	struct MouseTrigger : public ITriggerListener
	{
		MouseTrigger(MouseButton Button = LMB) : ITriggerListener(Events::InputGroup), Button(Button), State(false) {};
		MouseButton Button;
		bool State;
		inline bool Triggered() override { return State; };
		bool Receive(Events::IEvent *event) override;
	};
}

namespace Listener
{
	struct KeyListener : public Events::IEventListener
	{
		KeyListener() : IEventListener({ Events::KeyEvent }), KeyStates() {};
		KeyListener(size_t count, Input::Key *codes);
		KeyListener(std::vector<Input::Key> codes) : KeyListener(codes.size(), codes.data()) {}
		KeyListener(std::initializer_list<Input::Key> codes) : KeyListener(std::vector<Input::Key>(codes.begin(), codes.end())) {}
		template<size_t size>
		KeyListener(Input::Key(&codes)[size]);

		std::unordered_map<Input::Key, bool> KeyStates;
		virtual bool Receive(Events::IEvent *event) override;
		bool StateOf(Input::Key code);

		virtual ~KeyListener() {};
	};
	template<size_t size>
	inline KeyListener::KeyListener(Input::Key(&codes)[size]) : IEventListener(std::vector<std::wstring>(1, L"Key")), KeyStates()
	{
		for (auto& code : codes)
		{
			KeyStates[code] = false;
		}
	}

	struct MouseListener : public Events::IEventListener
	{
		MouseListener(double x = 0.0, double y = 0.0, double width = 1.0, double height = 1.0);

		void UpdateArea(DOUBLE4 Area);

		virtual bool Receive(Events::IEvent *event) override;

		void Anchor(CommonResources *resources);
		DOUBLE2 DeltaAbs(CommonResources *resources);
		DOUBLE2 DeltaAbsAnchor(CommonResources *resources);
		DOUBLE2 DeltaTriggered(CommonResources *resources);
		DOUBLE2 DeltaIfTriggered(CommonResources *resources);

		virtual ~MouseListener() {};
	protected:
		DOUBLE4 Area; // X, Y = TopLeft, Z/W = Width/Height
		DOUBLE2 InverseDimensions;
		PointU Mouse;
	};

	struct Checker : public Events::IEventListener
	{
		Checker(Events::IEvent *trigger) : IEventListener(std::vector<Events::Event>(1, trigger->Type)), Trigger(trigger), HasBeenOn(false) {};
		Checker(std::unique_ptr<Events::IEvent> trigger) : IEventListener(std::vector<Events::Event>(1, trigger->Type)), Trigger(std::move(trigger)), HasBeenOn(false) {};
		bool HasBeenOn;
		inline bool Check() {	bool Temp = HasBeenOn; 
								HasBeenOn = false; 
								return Temp;			};

		bool Receive(Events::IEvent *event) override;

	private:
		const std::unique_ptr<Events::IEvent> Trigger;
	};
}


namespace Event
{
	struct KeyInput : public Events::IEvent
	{
		constexpr static Events::Event MyType = Events::Event::KeyEvent;
		constexpr KeyInput(UINT keycode = 0x00, bool state = false, unsigned int frameid = 0) : IEvent(MyType, frameid), KeyCode(keycode), State(state) {};
		virtual bool operator==(Events::IEvent *other) override;
		Input::Key KeyCode;
		bool State; // True = key down, false = key up
	};

	struct MouseMove : public Events::IEvent
	{
	protected:
		constexpr MouseMove(int x, int y, Events::Event type, unsigned int frameid = 0) : IEvent(type, frameid), _X(x), _Y(y) {}
	public:
		constexpr static Events::Event MyType = Events::Event::MouseMoveEvent;
		constexpr MouseMove(int x, int y, unsigned int frameid = 0) : IEvent(MyType, frameid), _X(x), _Y(y) {};
		virtual bool operator==(Events::IEvent *other) override;
		int _X; // In Pixels
		int _Y; // In Pixels
		virtual ~MouseMove() {};
	};

	struct MouseButton : public MouseMove
	{
		constexpr static Events::Event MyType = Events::Event::MouseButtonEvent;
		constexpr MouseButton(int x, int y, ::MouseButton button, bool state, unsigned int frameid = 0) : MouseMove(x, y, MyType, frameid), Button(button), State(state) {};
		virtual bool operator==(Events::IEvent *other) override;
		
		::MouseButton Button;
		bool State;
	};

	struct MouseWheel : public MouseMove
	{
		constexpr static Events::Event MyType = Events::Event::MouseScrollEvent;
		constexpr MouseWheel(int x, int y, int scrollage, unsigned int frameid = 0) : MouseMove(x, y, MyType, frameid), Scrollage(scrollage) {};
		virtual bool operator==(Events::IEvent *other) override;
		int Scrollage;
	};

	struct MouseWheelButton : public MouseMove
	{
		constexpr static Events::Event MyType = Events::Event::MouseWheelButtonEvent;
		constexpr MouseWheelButton(int x, int y, bool state, unsigned int frameid = 0) : MouseMove(x, y, MyType, frameid), State(state) {}
		bool operator==(Events::IEvent *other) override;
		bool State;
	};
}

namespace Input
{
	struct KeyTimeListener : public Events::IEventListener
	{
	private:
		std::unordered_map<size_t, GameTimer> KeyTimes;
	public:
		KeyTimeListener() : IEventListener({ Events::KeyEvent }) {}
		KeyTimeListener(std::vector<Input::Key> keys) : IEventListener({ Events::KeyEvent }) { for (auto& key : keys) KeyTimes[key]; }

		void Tick();
		[[nodiscard]] inline double DeltaTime(Input::Key Key) { auto it = KeyTimes.find(Key); if (it != KeyTimes.end()) return it->second.DeltaTime(); else return 0.0; }

		[[nodiscard]] bool Add(Input::Key key); // Creates timer for key, returns false if there was already timer for the key
		[[nodiscard]] bool Remove(Input::Key key); // Removes timer associated with key, returns false if it could not find the timer

		bool Receive(Events::IEvent *event) override;
	};

	struct KeyPressListener : public Events::IEventListener
	{
	private:
		size_t Key = 0ull;
		bool Pressed = false;
	public:
		KeyPressListener(size_t key) : IEventListener({ Events::KeyEvent }), Key(key) {}

		virtual bool Receive(Events::IEvent * event) override;

		inline bool SetKey(size_t newKey)
		{
			Key = newKey;
			return BeenPressed();
		}

		[[nodiscard]] inline bool BeenPressed()
		{
			bool tmp = Pressed;
			Pressed = false;
			return tmp;
		}
	};
}