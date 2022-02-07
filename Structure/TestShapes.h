#pragma once

#include "BasicShapes.h"
#include <BulletDynamics/Character/btKinematicCharacterController.h>

namespace Test
{
	class IControllerState;

	class Controller : public G1::IShape, public btKinematicCharacterController, public Events::IEventListenerT<Events::KeyEvent, Events::WindowFocusEvent, Events::MouseButtonEvent, Events::AfterPhysicsEvent>
	{
		friend class IControllerState;
		std::unique_ptr<IControllerState> m_CurrentState;
		std::unique_ptr<IControllerState> m_TargetState;

		std::unique_ptr<btConvexShape> ControllerShape;
		std::shared_ptr<btPairCachingGhostObject> ControllerGhost;

		Pointer::selfish_ptr<Camera> Cam;

		// Cached stuff
		btVector3 m_Look = { 0.f, 0.f, 0.f };
		btVector3 m_LookHorizontal = { 0.f, 0.f, 0.f };
		btVector3 m_Right = { 0.f, 0.f, 0.f };
		btVector3 m_RightHorizontal = { 0.f, 0.f, 0.f };

		bool m_mouse_locked = false;

		void DoCam();
		void UpdateCache();
	public:
		Controller(G1::IShapeThings things);
		virtual ~Controller();

		virtual bool Receive(Events::IEvent *e) override;
		virtual bool Receive(Event::KeyInput *key) override;
		virtual bool Receive(Event::MouseButton *mb) override;
		virtual bool Receive(Event::AfterPhysicsEvent *post) override;
		virtual bool Receive(Event::WindowFocusEvent *win) override;

		virtual void BeforeDraw() override;
		virtual void AfterDraw() override;
	};

	class IControllerState : public Events::IEventListener
	{
	private:
		Controller *m_control;
	protected:

		// Observers
		const Camera &GetCamera() const;
		btVector3 GetLook() const;
		btVector3 GetHorizontalLook() const;
		btVector3 GetRight() const;
		btVector3 GetHorizontalRight() const;
		btVector3 GetPosition() const;
		G1::IGSpace *GetGSpace() const;

		btScalar GetTargetUpdateInterval() const;

		bool CanJump() const;

		// Modifiers
		void ChangeState(std::unique_ptr<IControllerState> newstate);

		void SetWalkDirection(const btVector3 &dir);
		void Jump(const btVector3 &impulse);

		// WIP
		void DisableGravity();
		void EnableGravity();
		bool HasGravity();

	public:
		IControllerState(Controller *control, std::vector<Events::Event> events) : Events::IEventListener(events), m_control(control) {} 
		virtual ~IControllerState() {}

		virtual void Reset() = 0;

		virtual void BeforePhysics(btScalar dt) = 0;
		//virtual void AfterPhysics() = 0;
	};

	class WalkingState : public IControllerState
	{


		float m_WalkPowerBase = 2.f; // in m/s
		float m_SprintMagnitude = 1.4f; // in m/s
		float m_TotalWalkSpeed = 2.f; // in m/s
		float m_forwardness = 0.f, m_rightness = 0.f;
		bool m_forward = false, m_back = false, m_left = false, m_right = false;
		bool m_sprint = false;

	public:
		WalkingState(Controller *control) : IControllerState(control, { Events::KeyEvent }) {}
		virtual ~WalkingState() {}

		virtual bool Receive(Events::IEvent *e) override;
		virtual bool Receive(Event::KeyInput *key) override;

		virtual void Reset() override;

		virtual void BeforePhysics(btScalar dt) override;
		virtual void AfterPhysics();
	};

	struct CeilTestStuff
	{
		floaty3 From;
		float Height = 1.f; // How far to test
		float LowerAmount = 0.01f; // Start ray from this much lower than the From vector
		bool UseRay = true;
	};

	bool CeilingTest(Controller *control, CeilTestStuff stuff); // TODO this thing
}