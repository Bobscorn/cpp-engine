#pragma once

#include "Events.h"

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
		KeyTrigger(Input::Key code = KEY_E) : ITriggerListener(Events::InputGroup), KeyCode(code), KeyState(false) {};
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