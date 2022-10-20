#pragma once

#include "Helpers/GSpaceHelper.h"
#include "Helpers/BulletHelper.h"

#include "BasicShapes.h"
#include "BulletBitmasks.h"

#include "Systems/Audio/Audio.h"

#include <BulletCollision/CollisionDispatch/btGhostObject.h>
#include <BulletDynamics/Character/btKinematicCharacterController.h>

#include <list>
#include <memory>

namespace Perviousity
{
	namespace Shapes
	{
		struct IPickupAble;

		struct PlatformMaterial
		{
			PlatformMaterial(Material mat) : OpaqueVersion(std::make_shared<Material>(mat)), TransparentVersion(std::make_shared<Material>(mat)), Lost(false) { TransparentVersion->DiffuseColor.w = 0.3f; }
			std::shared_ptr<Material> OpaqueVersion;
			std::shared_ptr<Material> TransparentVersion;
			bool Lost = false;
		};
	}

	namespace Player
	{
		struct CeilTestStuff
		{
			floaty3 From;
			float Height = 1.f; // How far to test
			float LowerAmount = 0.01f; // Start ray from this much lower than the From vector
			bool UseRay = true;
		};

		class IControllerState;
		
		struct PlayerStuff
		{
			floaty3 InitialPosition = { 0.f, 1.f, 0.f };
			floaty3 Direction = { 0.f, 0.f, -1.f };
		};

		extern const float DefaultOutOfBounds;

		// Rigid Body based player controller
		struct PerviousPlayer : G1I::PhysicsShape, Events::IEventListenerT<Events::WindowFocusEvent, Events::ResizeWindowEvent, Events::AfterPhysicsEvent, Events::MouseButtonEvent, Events::KeyEvent>
		{
			PerviousPlayer(G1::IGSpace* container, CommonResources* resources, PlayerStuff stuff = PlayerStuff{});
			~PerviousPlayer();

			void BeforeDraw() override;
			void AfterDraw() override;


			// Puzzle Methods

			inline bool IsOnGround() const { return m_OnGround; }
			inline floaty3 GetPosition() const { return Cam->GetPosition(); }
			inline floaty3 GetOrientation() const { return Cam->GetLook(); }

			//floaty3 SetPosition() const;

			// Make Player work methods

			// IEventListener/IAttachable methods
			bool Receive(Events::IEvent *event) override;
			bool Receive(Event::WindowFocusEvent *e) override;
			bool Receive(Event::AfterPhysicsEvent *e) override;
			bool Receive(Event::KeyInput *e) override;
			bool Receive(Event::MouseButton *e) override;

			bool Receive(Event::ResizeEvent *e) override;

			inline std::string GetName() const override { return Name; }

			//static std::unique_ptr<btCapsuleShape> Shape;
			//std::shared_ptr<btRigidBody> RigidBody;
			Pointer::selfish_ptr<Camera> Cam;

			// Merge stuff

			friend class IControllerState;
			friend bool CeilingTest(PerviousPlayer *player, CeilTestStuff stuff);
			std::unique_ptr<IControllerState> m_CurrentState;
			std::unique_ptr<IControllerState> m_TargetState;

			void ShootPewPewRay();
			bool ShootInteractRay();
			void SpiritMode();
			void NormalMode();
			void Jump(const btVector3 & jump);

			bool CanJump();

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

			PlayerStuff m_Stuff;

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
			static std::unique_ptr<btCapsuleShape> SweepCapsule;
			static std::unique_ptr<btCylinderShape> Capsule;

			Pointer::f_ptr<Shapes::IPickupAble> m_PickedUp;
			std::vector<Perviousity::Shapes::IPickupAble*> m_PickupAblesOnTopOf;
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

		bool CeilingTest(PerviousPlayer *player, CeilTestStuff stuff);

		class IControllerState : public Events::IEventListener
		{
		private:
			PerviousPlayer *m_control;
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
			bool Crouching() const;

			// Modifiers
			void ChangeState(std::unique_ptr<IControllerState> newstate);

			void SetWalkDirection(const btVector3 &dir);
			void Jump(const btVector3 &impulse);
			void Crouch();
			void StandUp();
			void SetCrouchState(bool state);

			void StampGround(); // Play footstep sound

			// WIP
			void DisableGravity();
			void EnableGravity();
			bool HasGravity();

		public:
			IControllerState(PerviousPlayer *control, std::vector<Events::Event> events) : Events::IEventListener(events), m_control(control) {}
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
			WalkingState(PerviousPlayer *control) : IControllerState(control, { Events::KeyEvent }) {}
			virtual ~WalkingState() {}

			virtual bool Receive(Events::IEvent *e) override;
			virtual bool Receive(Event::KeyInput *key) override;

			virtual void Reset() override;

			virtual void BeforePhysics(btScalar dt) override;
			virtual void AfterPhysics();
		};

	}

	namespace Activators
	{
		std::shared_ptr<Perviousity::Shapes::PlatformMaterial> GetActivatorMaterial();

		struct IActivatable : virtual BulletHelp::INothingInterface
		{
			virtual ~IActivatable() {}

			virtual void Activate() = 0;

			inline virtual void KeepActivated(size_t updateID) { m_LastUpdate = updateID; };

			inline virtual bool AllowPlayerActivate() const { return true; }

		protected:
			size_t m_LastUpdate = 0ull;
		};

		struct StandButtonThings
		{
			Pointer::f_ptr<IActivatable> Activatable;
			Matrixy4x4 Transform;
			btVector3 Normal;
			bool Is_End = false;

			StandButtonThings &RemakeWithNewMatrix();
		};

		struct StandButton : G1::IShape, IActivatable
		{
			StandButton(G1::IShapeThings ting, StandButtonThings ting2 = { nullptr, Matrixy4x4::Identity() });
			~StandButton() { m_SpecialColor->Lost = true; }

			inline virtual void BeforeDraw() override {}
			inline virtual void AfterDraw() override {}

			virtual void Activate() override;

			struct EnsureFailure : std::exception
			{
				const char *what() const noexcept override
				{
					return "Failed to Ensure Stand button Geometry";
				}
			};
		protected:
			StandButtonThings m_Things;
			Matrixy4x4 RedWorld;
			Matrixy4x4 CaseWorld;
			Matrixy4x4 StandWorld;

			std::shared_ptr<btRigidBody> StaticBody;
			std::shared_ptr<btGhostObject> ButtonGhost;

			btDefaultMotionState BodyState;
			Audio::ALSource m_SoundSource;

			static std::weak_ptr<GeoThing> s_StandGeo;
			static std::weak_ptr<GeoThing> s_RedGeo;
			static std::weak_ptr<GeoThing> s_CaseGeo;
			static std::weak_ptr<GeoThing> s_SpecialGeo;
			static std::weak_ptr<Material> s_StandMat;
			static std::weak_ptr<Material> s_RedMat;
			static std::weak_ptr<Material> s_CaseMat;
			std::shared_ptr<GeoThing> m_StandGeo;
			std::shared_ptr<GeoThing> m_RedGeo;
			std::shared_ptr<GeoThing> m_CaseGeo;
			std::shared_ptr<GeoThing> m_SpecialGeo;
			std::shared_ptr<Material> m_StandMat;
			std::shared_ptr<Material> m_RedMat;
			std::shared_ptr<Material> m_CaseMat;
			std::shared_ptr<Perviousity::Shapes::PlatformMaterial> m_SpecialColor;

			//static Damn::RawNode StandButtonStand;
			//static Damn::RawNode StandButtonRed;
			//static Damn::RawNode StandButtonCase;

			static Matrixy4x4 StandMatrix;
			static Matrixy4x4 RedMatrix;
			static Matrixy4x4 CaseMatrix;

			static std::shared_ptr<btCylinderShape> StaticMesh;
			static std::shared_ptr<btSphereShape> GhostShape;

			constexpr static Material CaseConstMaterial = { { 0.f, 0.f, 0.f, 0.f }, { 0.1f, 0.1f, 0.08f, 1.f}, { 0.5f, 0.5f, 0.5f, 1.f }, { 0.13f, 0.13f, 0.13f, 50.f }, 1.f, 0u };
			constexpr static Material RedConstMaterial = { {0.f, 0.f, 0.f, 0.f}, {0.1f, 0.01f, 0.01f, 1.f}, {0.35f, 0.1f, 0.1f, 1.f}, {0.17f, 0.17f, 0.17f, 50.f}, 1.f, 0u };

			void EnsureGeometry(Matrixy4x4 trans);
			static void EnsureShapes();
			Matrixy4x4 GetBodyState();
		};

		struct PressurePadThings
		{
			Pointer::f_ptr<IActivatable> Activatable;
			Matrixy4x4 Transform;
		};

		struct PressurePad : G1::IShape, IActivatable
		{
			PressurePad(G1::IShapeThings ting, PressurePadThings ting2);
			~PressurePad() { m_SpecialMat->Lost = true; };

			virtual void BeforeDraw() override;
			virtual void AfterDraw() override;

			virtual void Activate() override;
						
		protected:

			PressurePadThings m_PadThings;

			std::shared_ptr<btRigidBody> StaticBody;
			std::shared_ptr<btGhostObject> PadGhost;

			static std::weak_ptr<GeoThing> s_BaseGeo;
			static std::weak_ptr<GeoThing> s_PressureGeo;
			static std::weak_ptr<GeoThing> s_SpecialGeo;
			static std::weak_ptr<Material> s_BaseMat;
			static std::weak_ptr<Material> s_PressureMat;
			std::shared_ptr<GeoThing> m_BaseGeo;
			std::shared_ptr<GeoThing> m_PressureGeo;
			std::shared_ptr<GeoThing> m_SpecialGeo;
			std::shared_ptr<Material> m_BaseMat;
			std::shared_ptr<Material> m_PressureMat;

			std::shared_ptr<Perviousity::Shapes::PlatformMaterial> m_SpecialMat;

			//static Damn::RawNode PadBaseNode;
			//static Damn::RawNode PadPressureNode;

			static std::shared_ptr<btBoxShape> StaticShape;
			static std::shared_ptr<btBoxShape> GhostShape;

			void EnsureGeometry();
			static void EnsureShapes();
		};

		// Shape that initiates the end of a level (does not know how the level ends just tells stuff that it has ended)
		// Does not contain any geometry, physical bodies or ghosts or anything
		struct Ender : G1::IShape, IActivatable
		{
			Ender(G1::IShapeThings ting);
			~Ender() {}

			virtual void BeforeDraw() override;
			virtual void AfterDraw() override;

			virtual void Activate() override;
		};
	}

	namespace Shapes
	{
		struct IPerviousThing;

		struct IPerviousThing : virtual BulletHelp::INothingInterface, Activators::IActivatable
		{
			virtual ~IPerviousThing() {}

			virtual void TurnOff() = 0;
			virtual void TurnOn() = 0;
			virtual bool Enabled() = 0;

			virtual void Activate() override;

			virtual void AddMaterial(std::shared_ptr<PlatformMaterial> mat) = 0;
		};


		struct PerviousWall;
		struct ImperviousWall;
		struct PerviousFloor;
		struct ImperviousFloor;


		enum WallDirection
		{
			Forward = 0, // Default, 0 90* rotations
			NegativeZ = 0,
			Right = 1, // 1 90* Rotation
			PositiveX = 1, 
			Backward = 2, // 2 90* Rotations (technically no rotation for uniform walls)
			PositiveZ = 2,
			Left = 3, // 3 90* Rotations
			NegativeX = 3
		};

		enum PerviousFormType
		{
			WallForward = 0,
			WallNorth = 0,
			WallNegativeZ = 0,
			WallRight = 1,
			WallEast = 1,
			WallPositiveX = 1,
			WallBackward = 2,
			WallSouth = 2,
			WallPositiveZ = 2,
			WallLeft = 3,
			WallWest = 3,
			WallNegativeX = 3,
			Floor = 4,
		};

		inline bool IsWall(PerviousFormType type) { return type < Floor; }
		inline bool IsFloor(PerviousFormType type) { return type == Floor; }

		extern std::shared_ptr<btCollisionShape> WallShape;
		extern std::unique_ptr<btCollisionShape> FloorShape;

		constexpr Material OpaqueMaterialData = Material{ {0.f, 0.f, 0.f, 1.f}, {0.f, 0.f, 0.f, 1.f}, {0.4f, 0.4f, 0.4f, 1.f}, {0.55f, 0.55f, 0.55f, 1.5f}, 1.f, 0 };
		constexpr Material TransparentMaterialData = Material{ {0.f, 0.f, 0.f, 1.f}, {0.f, 0.f, 0.f, 1.f}, {0.4f, 0.4f, 0.4f, 0.4f}, {0.f, 0.f, 0.f, 1.f}, 1.f, 0 };
		extern std::shared_ptr<Material> g_OpaqueMaterial;
		extern std::shared_ptr<Material> g_TransparentMaterial;

		constexpr floaty3 WallExtents = { 0.984375f, 0.984375f, 0.1f };
		constexpr floaty3 FloorExtents = { 0.984375f, 0.1f, 0.984375f };


		template<class T>
		void TurnOffPlatform(T *obj, CommonResources *resources)
		{
			(void)resources;
			if constexpr (std::is_same<T, PerviousWall>::value)
				obj->TurnOff();
			else
				obj->m_Wall->TurnOff();
		}

		struct PerviousThings
		{
			Matrixy4x4 Transform;
			btVector3 Normal;
			bool Activated = false;
			BulletHelp::NothingHolder *ExternalHolder = nullptr;
		};


		extern std::shared_ptr<PlatformMaterial> DefaultMaterial;

		struct PerviousWall : virtual G1::IShape, IPerviousThing
		{
			PerviousWall(G1::IShapeThings ting, PerviousThings ting2);

			void BeforeDraw() override {}
			virtual void AfterDraw() override {
				CheckMaterials();
				if (m_LastUpdate == *mResources->UpdateID)
				{
					if (Enabled())
					{
						if (m_WasOn)
						{
							MakeOff();
							m_WasOn = false;
						}
					}
					else if (!m_WasOn)
					{
						MakeOn();
						m_WasOn = true;
					}
				}
				else
				{
					if (Enabled())
					{
						if (!m_WasOn)
						{
							MakeOn();
							m_WasOn = true;
						}
					}
					else if (m_WasOn)
					{
						MakeOff();
						m_WasOn = false;
					}
				}
			}

			virtual void TurnOff() override;
			virtual void TurnOn() override;

			virtual bool Enabled() override;

			virtual void AddMaterial(std::shared_ptr<PlatformMaterial> mat) override;

		protected:
			void CheckMaterials();

			void EnsurePerviousResources(CommonResources *resources);

			static std::weak_ptr<GeoThing> s_PartGeos[4];
			static std::weak_ptr<GeoThing> s_OuterGeo;
			static std::weak_ptr<Material> s_DefaultMat;
			std::shared_ptr<GeoThing> m_PartGeos[4];
			std::shared_ptr<GeoThing> m_OuterGeo;
			std::shared_ptr<Material> m_DefaultMat;

			std::array<std::shared_ptr<PlatformMaterial>, 4ull> m_SpecialMats;

			// Does not change m_Things.Activated
			// Intended for use in AfterDraw to implement KeepActivated Functionality
			virtual void MakeOff();
			virtual void MakeOn();

			G1I::PhysicsBody *m_Wall = nullptr;
			G1I::RawRendyShape *m_Outer = nullptr;
			G1I::RawRendyShape *m_Parts[4] = { nullptr, nullptr, nullptr, nullptr };
			PerviousThings m_Things;
			bool m_WasOn = false;
		};

		struct ImperviousWall : virtual G1::IShape
		{
			ImperviousWall(G1::IShapeThings ting, PerviousThings ting2);

			void BeforeDraw() override {}
			void AfterDraw() override {}

		protected:
			G1I::CubeyShape *m_Wall = nullptr;
			PerviousThings m_Things;
		};

		struct PerviousFloor : virtual G1::IShape, IPerviousThing
		{
			PerviousFloor(G1::IShapeThings ting, PerviousThings ting2);

			void BeforeDraw() override {}
			void AfterDraw() override {}

			virtual void TurnOff() override;
			virtual void TurnOn() override;

			virtual bool Enabled() override;

		protected:
			G1I::CubeyShape *m_Floor = nullptr;
			PerviousThings m_Things;
		};

		struct ImperviousFloor : virtual G1::IShape
		{
			ImperviousFloor(G1::IShapeThings ting, PerviousThings ting2);

			void BeforeDraw() override {}
			void AfterDraw() override {}

		protected:
			G1I::CubeyShape *m_Floor = nullptr;
			PerviousThings m_Things;
		};

		// Override's TurnOff and TurnOn to disable player activation
		struct ActivateOnlyWall : PerviousWall
		{
		protected:
			void ActualTurnOff();
			void ActualTurnOn();

		public:
			ActivateOnlyWall(G1::IShapeThings ting, PerviousThings ting2);
			
			virtual void TurnOff() override;
			virtual void TurnOn() override;

			virtual void Activate() override;
		};



		struct GroupController;


		struct GroupObjectInfo
		{
			GroupController *Control;
			Matrixy4x4 Trans;
			btVector3 Normal;
		};


		struct GroupObjectInfoOld
		{
			GroupController *Control;
			floaty3 Position;
			PerviousFormType Type;

			inline explicit operator GroupObjectInfo() const 
			{ 
				switch (Type)
				{
				default:
				case PerviousFormType::Floor:
					return { Control, Matrixy4x4::Translate(Position), {0.f, 1.f, 0.f} };
				case PerviousFormType::WallNegativeX:
					return { Control, Matrixy4x4::Translate(Position), {-1.f, 0.f, 0.f} };
				case PerviousFormType::WallNegativeZ:
					return { Control, Matrixy4x4::Translate(Position), {0.f, 0.f, -1.f} };
				case PerviousFormType::WallPositiveX:
					return { Control, Matrixy4x4::Translate(Position), {1.f, 0.f, 0.f } };
				case PerviousFormType::WallPositiveZ:
					return { Control, Matrixy4x4::Translate(Position), {0.f, 0.f, 1.f} };
				}
			}
		};

		struct GroupPerviousObject;
		
		struct GroupPerviousObject : G1::IShape, IPerviousThing
		{
			friend struct GroupController;
			friend void TurnOffPlatform<GroupPerviousObject>(GroupPerviousObject *, CommonResources*);

			GroupPerviousObject(G1::IShapeThings ting, GroupObjectInfoOld info);
			GroupPerviousObject(G1::IShapeThings ting, GroupObjectInfo info, bool activated = false);

			void BeforeDraw() override {}
			void AfterDraw() override {}

			void TurnOn() override;
			void TurnOff() override;

			bool Enabled() override;

			virtual void AddMaterial(std::shared_ptr<PlatformMaterial> mat) override;

		protected:

			GroupObjectInfo Info;
			PerviousWall *m_Wall = 0;
		};

		struct GroupControllerInfo
		{
			size_t PlatformCount = 1ull; // If Inverted this is the max number of Opaque Platforms, otherwise max number of transparent platforms
			bool Inverted = false;
			bool EnableAbility = true;
			bool DisableAbility = true;
		};

		template<class T, class ... Args>
		struct GroupDefferedSpecialize
		{
			static void AddChild(GroupController *guy, Args... args);
		};

		struct GroupController : G1::IShape
		{
			template<class T, class ... Args>
			friend struct GroupDefferedSpecialize;

			GroupController(G1::IShapeThings ting, GroupControllerInfo info);

			void BeforeDraw() override {}
			void AfterDraw() override {}

			void TryTurnOff(GroupPerviousObject *obj);
			void TryTurnOn(GroupPerviousObject *obj);

			void SetDisableAbility(bool val);
			void SetEnableAbility(bool val);

			template<class T, class ... Args>
			void AddChild(Args... args)
			{
				GroupDefferedSpecialize<T, Args...>::AddChild(this, args...);
			}

		protected:
			std::list<GroupPerviousObject*>::iterator InTargets(GroupPerviousObject *obj);

			GroupControllerInfo Info;
			std::list<GroupPerviousObject*> Targets; // Normal: Targets are Turned Off platforms, Inverted: Targets are turned on
		};

		template<>
		struct GroupDefferedSpecialize<GroupPerviousObject, floaty3, PerviousFormType>
		{
			static void AddChild(GroupController *guy, floaty3 pos, PerviousFormType type)
			{
				auto * p = new GroupPerviousObject({ guy->Container, guy->mResources }, { guy, pos, type });
				guy->G1::IShape::AddChild((G1::IShape*)p);
				if (guy->Info.Inverted)
				{
					if (guy->Targets.size() >= guy->Info.PlatformCount)
					{
						TurnOffPlatform(p, guy->mResources);
					}
					else
					{
						guy->Targets.emplace_back(p);
					}
				}
			}
		};

		template<>
		struct GroupDefferedSpecialize<GroupPerviousObject, Matrixy4x4, btVector3, bool>
		{
			static void AddChild(GroupController *guy, Matrixy4x4 trans, btVector3 normal, bool pre_activated)
			{
				auto *p = new GroupPerviousObject({ guy->Container, guy->mResources }, { guy, trans, normal }, pre_activated);
				guy->G1::IShape::AddChild((G1::IShape *)p);
			}
		};

		template<class T, class ... Args>
		inline void GroupDefferedSpecialize<T, Args...>::AddChild(GroupController * guy, Args ...args)
		{
			guy->G1::IShape::AddChild<T, Args...>(std::forward(args)...);
		}



		struct PairedPerviousObject;

		struct PairedThings
		{
			Matrixy4x4 Trans;
			btVector3 Normal;
			Pointer::f_ptr<PairedPerviousObject> Pair = nullptr;
			bool NoActivate = false;
			bool Copy = false;
			size_t LastActivateID = 0ull;
		};

		struct PairedPerviousObject : G1::IShape, IPerviousThing
		{
			PairedPerviousObject(G1::IShapeThings ting, PairedThings ass, bool activated);

			void SetPair(Pointer::f_ptr<PairedPerviousObject> p);

			void BeforeDraw() override;
			void AfterDraw() override;

			void TurnOn() override;
			void TurnOff() override;

			bool Enabled() override;

			void DisablePlayerActivation();
			void EnablePlayerActivation();

			virtual void KeepActivated(size_t updateid) override;
			virtual bool AllowPlayerActivate() const override;

			virtual void AddMaterial(std::shared_ptr<PlatformMaterial> mat) override;

		protected:

			void SecretTurnOff();
			void SecretTurnOn();

			PairedThings Info;

			PerviousWall *m_Wall;
			size_t m_PairLastUpdate = 0ull;
		};

		struct ToggleChain;
		struct CopyChain;
		struct InvertChain;

		struct IChainedPerviousObject : G1::IShape, IPerviousThing
		{
		private:
			friend struct ToggleChain;
			friend struct CopyChain;
			friend struct InvertChain;
			IChainedPerviousObject(G1::IShapeThings shapedesc, floaty3 pos, PerviousFormType type, IChainedPerviousObject *link = nullptr);
		public:
			virtual ~IChainedPerviousObject() {}

			void SetLink(IChainedPerviousObject *link);

			virtual void BeforeDraw() override {}
			virtual void AfterDraw() override {}

			virtual void TurnOn() override;
			virtual void TurnOff() override;

			virtual bool Enabled() override;

		protected:
			virtual void DoAction(bool turnon) = 0;

			G1I::CubeyShape *Body = nullptr;
			floaty3 Position;
			PerviousFormType Type;

			IChainedPerviousObject *Link = nullptr;
		};

		struct ToggleChain : IChainedPerviousObject
		{
			ToggleChain(G1::IShapeThings shapedesc, floaty3 pos, PerviousFormType type, IChainedPerviousObject *link = nullptr) : IChainedPerviousObject(shapedesc, pos, type, link) {}

		protected:
			inline virtual void DoAction(bool turnon) override
			{
				(void)turnon;
				if (Link)
					if (Link->Enabled())
						Link->TurnOff();
					else
						Link->TurnOn();
			}
		};

		struct CopyChain : IChainedPerviousObject
		{
			CopyChain(G1::IShapeThings shapedesc, floaty3 pos, PerviousFormType type, IChainedPerviousObject *link = nullptr) : IChainedPerviousObject(shapedesc, pos, type, link) {}

		protected:
			inline virtual void DoAction(bool turnon) override
			{
				if (Link)
					if (turnon)
					{
						if (!Link->Enabled())
							Link->TurnOn();
					}
					else
						if (Link->Enabled())
							Link->TurnOff();
			}
		};

		struct InvertChain : IChainedPerviousObject
		{
			InvertChain(G1::IShapeThings shapedesc, floaty3 pos, PerviousFormType type, IChainedPerviousObject *link = nullptr) : IChainedPerviousObject(shapedesc, pos, type, link) {}

		protected:
			inline virtual void DoAction(bool turnon) override
			{
				if (Link)
				{
					if (turnon)
					{
						if (Link->Enabled())
							Link->TurnOff();
					}
					else
						if (!Link->Enabled())
							Link->TurnOn();
				}
			}
		};


		struct CubeyBoiThings
		{
			Matrixy4x4 Transform;
			float Mass = 1.f;
		};

		struct IPickupAble : BulletHelp::INothingInterface
		{
			virtual btVector3 GetPosition() const = 0;
			virtual btTransform GetTransform() const = 0;
			virtual void SetPosition(btVector3 pos) = 0;
			virtual void SetVelocity(btVector3 vel) = 0;

			virtual void ApplyTorque(btVector3 torque) = 0;

			virtual Pointer::f_ptr<IPickupAble> GetMe() = 0;
			virtual btRigidBody* GetBody() = 0;
		};

		struct CubeyBoi : G1::IShape, btMotionState, IPickupAble
		{
			CubeyBoi(G1::IShapeThings tings, CubeyBoiThings things);
			~CubeyBoi();

			virtual void BeforeDraw() override;
			virtual void AfterDraw() override;

			virtual void setWorldTransform(const btTransform &trans) override;
			virtual void getWorldTransform(btTransform &out) const override;

			virtual btVector3 GetPosition() const override;
			virtual btTransform GetTransform() const override;
			virtual void SetPosition(btVector3 pos) override;
			virtual void SetVelocity(btVector3 vel) override;

			virtual void ApplyTorque(btVector3 torque) override;

			virtual Pointer::f_ptr<IPickupAble> GetMe() override;
			virtual btRigidBody* GetBody() override;

		protected:
			CubeyBoiThings m_CubeThings;
			Matrixy4x4 m_ActiveTransform;
			size_t m_DrawCallKey = 0ull;

			constexpr static float DefaultOutOfBounds = -50.f;

			std::shared_ptr<btRigidBody> m_CubeBody;

			static std::shared_ptr<btBoxShape> CubeShape;
		};
	}
}