#pragma once

#include "VoxelAbility.h"

#include "Helpers/BulletHelper.h"

#include "Systems/Events/EventsBase.h"
#include "Systems/Input/InputAttachment.h"

#include "Structure/BasicGSpace.h"
#include "Systems/Audio/Audio.h"

#include <LinearMath/btVector3.h>

namespace Voxel
{
	struct VoxelPlayerStuff
	{
		floaty3 InitialPosition = { 0.f, 2.f, 0.f };
		floaty3 InitialDirection = { 0.f, 0.f, -1.f };
		float MovementSpeed = 3.f; // In m/s
	};

	// Rigid Body based player controller
	struct VoxelPlayer : G1::IShape, Events::IEventListenerT<Events::WindowFocusEvent, Events::ResizeWindowEvent, Events::AfterPhysicsEvent>, public InputAttach::IAttachable, Entity
	{
		VoxelPlayer(G1::IShapeThings things, VoxelWorld *world, VoxelPlayerStuff stuff);
		~VoxelPlayer();

		void BeforeDraw() override;
		void AfterDraw() override;

		virtual void TakeKnockbackDirect(floaty3 knock_back) override;
		virtual bool HitByProjectile(Projectile *proj, floaty3 hit_point) override;

		virtual floaty3 GetCentre() override;

		// Puzzle Methods

		inline bool IsOnGround() const { return m_OnGround; }
		inline floaty3 GetPosition() const { return Cam->GetPosition(); }
		inline floaty3 GetOrientation() const { return Cam->GetLook(); }

		void SetPosition(floaty3 newPos);
		void SetVelocity(floaty3 newVel);

		// Make Player work methods

		// IEventListener/IAttachable methods
		bool Receive(Event::KeyInput *e) override;
		bool Receive(Event::MouseButton *e) override;

		void Superseded() override;

		bool Receive(Events::IEvent *event) override;
		bool Receive(Event::WindowFocusEvent *e) override;
		bool Receive(Event::AfterPhysicsEvent *e) override;

		bool Receive(Event::ResizeEvent *e) override;

		inline std::string GetName() const override { return Name; }

		//static std::unique_ptr<btCapsuleShape> Shape;
		//std::shared_ptr<btRigidBody> RigidBody;
		Pointer::selfish_ptr<Camera> Cam;

		void ShootTestRay(bool destroy);
		bool ShootInteractRay();
		void SpiritMode();
		void NormalMode();
		void Jump(const btVector3 &jump);

		struct RayReturn
		{
			BulletHelp::NothingHolder *hold;
			btVector3 rayFrom;
			btVector3 rayTo;
			btVector3 hitPoint;
			btVector3 normal;
		};

		RayReturn FirstHit(float raylength);

		void TmpRayThing(bool destroy);
		void TmpEntityThing(RayReturn ray, Entity *entity);
		void BreakBlock(RayReturn ray);
		void PlaceBlock(RayReturn ray);
				
		// Walking Stuff
		float m_SprintMagnitude = 1.4f; // in m/s
		float m_TotalWalkSpeed = 2.f; // in m/s
		bool m_sprint = false;

		void SetCrouchState(bool state);
		void DoWalkFrame(btScalar dt);
		void ResetState();

		bool CanJump();

		//void getWorldTransform(btTransform &t) const override;
		//void setWorldTransform(const btTransform &t) override;

		virtual void Die() override;

	protected:
		void DoCam();
		void DoPhys();
		void DoPickup();
		void UpdateCache();
		void DoSound();
		void TestForOutOfBounds();

		void ResetPickup();

		void UpdateRayCache();
		void DoFeetRays();
		btVector3 GetRotatedForce(btVector3 in);

		void DisableGroundPull();
		void EnableGroundPull();

		constexpr static float CrouchForce = 75.f;
		void ApplyCrouchForce();

		void LockMouse();
		void UnlockMouse();
		void StampOnGround(); // Play the footstep sound

		void DoPlaceBlock();
		void PlaceBlock();

		float GetDeterminant(btCollisionWorld *world, btVector3 *deepestnormal = nullptr);
		float GetSpringFactor(btScalar YVel);
		float GetNewYVelSpring(btScalar current, btScalar deltatime);
		float GetNewYStage1(btScalar current, btScalar time, btScalar &out); // Returns excess time
		float GetNewYStage2(btScalar current, btScalar time, btScalar &out); // Returns excess time
		float GetNewYStage3(btScalar current, btScalar time, btScalar &out); // Returns excess time

		// Controller Movement/Body stuff
		constexpr static float CapsuleHeight = 0.6f;
		constexpr static float CapsuleRadius = .25f;
		constexpr static float CapsuleRadiusBonus = -0.015625f;
		constexpr static float LegLength = .65f;
		constexpr static float DeadZoneLength = .015625f;
		constexpr static float FeetLength = .25f;
		constexpr static float StubLength = .01f;
		constexpr static float RayFinalLength = LegLength + DeadZoneLength + FeetLength + StubLength;
		constexpr static float MaxPullDown = 0.04f;
		constexpr static float CrouchScale = 0.5f;
		constexpr static float LegRayAmount = LegLength / RayFinalLength;
		constexpr static float FeetRayStart = (LegLength + DeadZoneLength) / RayFinalLength;
		constexpr static float FeetRayEnd = FeetRayStart + FeetLength / RayFinalLength;
		constexpr static float MinPullDown = 0.01f;
		constexpr static float MinVelForMinPullDown = 0.25f;
		constexpr static float RayCircleScale = .75f;
		constexpr static float DefaultY = -(CapsuleHeight * .5f) - (CapsuleRadius * .5f);
		constexpr static float SpringK = 12000.f;
		constexpr static float SpringKSlowUp = 1000.f;
		constexpr static float MinSpringX = 0.0001f;
		constexpr static float CompleteHeight = (0.5f * CapsuleHeight + CapsuleRadius) - (DefaultY - RayFinalLength);

		constexpr static float RotateForceAmount = 0.25f; // 0.0-1.0
		constexpr static float ShootDistance = 5.f;
		constexpr static float InteractDistance = 3.5f;

		constexpr static btScalar Fatness = btScalar(50.0);
		constexpr static btScalar walkpower = btScalar(3.0);
		constexpr static btScalar airpower = btScalar(7.5) * Fatness;
		constexpr static btScalar LowFric = btScalar(0.0);
		constexpr static btScalar HighFric = btScalar(1.0);
		constexpr static btScalar SuperHighFric = btScalar(5.0);
		constexpr static float JumpPower = 5.f;
		constexpr static float FinalJumpForce = JumpPower * Fatness;

		constexpr static float MinGroundAngle = 45.f; // 0* means only if ground is perfectly flat will it be considered ground 90* means vertical slopes count too
		constexpr static float MinGroundDot = 0.57357643635104609610803191282616f;
		constexpr static float MaxGroundIntersection = 0.015f;

		VoxelPlayerStuff m_Stuff;
		std::string m_SelectedBlockName = "";

		// Sound
		constexpr static float FootstepInterval = 0.6f;
		GameTimer m_FootStepTimer;
		Audio::ALSource m_FeetSource;
		std::vector<Audio::ALBufferI> m_FootstepSounds;
		std::uniform_int_distribution<size_t> m_FootstepRandomizer;
		std::vector<Audio::ALBufferI> GetFootstepSounds();

		size_t Raycount = 3ull;
		std::vector<floaty3> m_RayOffset;
		btScalar m_ConvexOffset = btScalar(0.0);
		btVector3 m_DeepestNormal = { 0.f, -1.f, 0.f }; // Used by the slope rotater to smooth out slopes more
		GameTimer m_PullTimer; // Use a pull timer in case something pushes the body back to ground early (like a ceiling)

		constexpr static float MinPullStart = 0.25f; // Seconds before GroundPull can be re-enabled

		std::shared_ptr<btRigidBody> m_RigidBody;
		std::shared_ptr<btRigidBody> m_KinematicFeet;
		btDefaultMotionState m_FeetState;
		btTransform m_Trans;
		static std::unique_ptr<btCapsuleShape> SweepCapsule;
		static std::unique_ptr<btCylinderShape> Capsule;

		//Pointer::f_ptr<Shapes::IPickupAble> m_PickedUp;
		//std::vector<Perviousity::Shapes::IPickupAble *> m_PickupAblesOnTopOf;
		btScalar m_PickupOffset = 1.5f;
		constexpr static float m_MaxPickupDistance = 1.5f;
		constexpr static float s_PickupMoveFactor = 10.f;
		constexpr static float s_PickupSpinFactor = 20.f;


		btVector3 m_Look = { 0.f, 0.f, 0.f };
		btVector3 m_LookHorizontal = { 0.f, 0.f, 0.f };
		btVector3 m_Right = { 0.f, 0.f, 0.f };
		btVector3 m_RightHorizontal = { 0.f, 0.f, 0.f };

		floaty2 mouse_locked_location = { 0.f, 0.f }; // In OpenGL coords
		float forwardness = 0.f, rightness = 0.f;
		size_t LastJustFrameID = 0ull;

		bool forward = false, back = false, left = false, right = false;

		bool m_UseRayTest = false;

		bool m_OnGround = false;
		bool m_Crouched = false;
		bool m_MouseIsLocked = false;
		bool m_IsSpirit = false;
		bool m_DisableGroundPull = false;
		bool m_ReenablePullNextRequirement = false;
	};
}