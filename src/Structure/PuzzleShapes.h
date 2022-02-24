#pragma once

#include "BasicShapes.h"

#include "Helpers/MathHelper.h"

#include "Game/Progress.h"
#include "Systems/Timer/Timer.h"

namespace Puzzle
{

	namespace G1I
	{
		struct FirstRoom;
		struct SecondRoom;
		struct PuzzlePlayer;

		struct FirstPuzzlePlace : G1::IShape
		{
			FirstPuzzlePlace(G1::IGSpace *container, CommonResources *resources, IStageManager * man);

			void BeforeDraw() override {}
			void AfterDraw() override;

			inline std::string GetName() const override { return "First Puzzle Place"; }

		protected:
			IStageManager *Manager = nullptr;
			PuzzlePlayer *player = nullptr;
			FirstRoom *first = nullptr;
			SecondRoom *second = nullptr;
		};

		// Rigid Body based player controller
		struct PuzzlePlayer : ::G1I::PhysicsShape, Events::IEventListenerT<Events::WindowFocusEvent, Events::ResizeWindowEvent, Events::AfterPhysicsEvent, Events::MouseButtonEvent, Events::KeyEvent>
		{
			PuzzlePlayer(G1::IGSpace *container, CommonResources *resources);

			void BeforeDraw() override;
			void AfterDraw() override;


			// Puzzle Methods

			inline bool IsOnGround() const { return groundy; }
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
			
			static std::unique_ptr<btCapsuleShape> Shape;
			std::shared_ptr<btRigidBody> RigidBody;
			Pointer::selfish_ptr<Camera> Cam;

		protected:
			void LockMouse();
			void UnlockMouse();
			bool CheckGrounding(btDynamicsWorld *world);

			constexpr static float JumpCooldown = 0.15f;
			constexpr static float CapsuleHeight = 1.5f;
			constexpr static float CapsuleRadius = 0.5f;

			constexpr static btScalar walkpower = btScalar(1.5f);
			constexpr static btScalar airpower = btScalar(150.f);
			constexpr static float JumpPower = 50.f;

			GameTimer JumpTimer;
			floaty2 mouse_locked_location = { 0.f, 0.f }; // In OpenGL coords
			float forwardness = 0.f, rightness = 0.f;
			size_t LastJustFrameID = 0ull;

			bool forward = false, back = false, left = false, right = false;

			bool jump = false;
			bool wasinair = false;
			bool groundy = false;
			bool mouse_locked = false;
			bool outputcaminfo = true;
		};

		// First Puzzle room using 6 static bodies (1 - floor, 1 - roof, 4 - walls)
		// Tilting room
		struct FirstRoom : virtual ::G1::IShape, virtual FullResourceHolder
		{
			FirstRoom(G1::IGSpace *container, CommonResources *resources, PuzzlePlayer *player, IStageManager *man);

			void BeforeDraw() override {}
			void AfterDraw() override;

			inline bool ReadyForMore() const { return Finished; }

		protected:
			friend struct SecondRoom;
			bool XFacCloseEnough(float fac);
			bool YFacCloseEnough(float fac);
			void Win(); // Called when the player wins, calls Complete
			void Complete(); // Sets puzzle state to completed, different to Win because it may be loaded as completed
			void MoveOn();

			constexpr static float RoomRadius = 6.f;
			constexpr static float HalfThickness = 0.2f;
			constexpr static float RoomHeight = 5.f;
			constexpr static Material WallMaterial = { floaty4{0.1f, 0.1f, 0.1f, 1.f}, floaty4{0.f, 0.f, 0.f, 1.f}, floaty4{1.0f, 1.0f, 1.0f, 1.f}, floaty4{0.9f, 0.9f, 0.9f, 1.5f}, 1.f, 0 };
			constexpr static Material FlooroofMaterial = { floaty4{0.1f, 0.1f, 0.1f, 1.f}, floaty4{0.f, 0.f, 0.f, 1.f}, floaty4{1.0f, 1.0f, 1.0f, 1.f}, floaty4{0.9f, 0.9f, 0.9f, 1.5f}, 1.f, 0 };
			constexpr static float MaxDistance = 7.f;
			constexpr static float MaxRotation = 35.f * Math::DegToRadF;
			constexpr static float MinPosDistance = 1.f;
			constexpr static float MaxPosDistance = 4.f;
			constexpr static floaty3 FloorPosition = { 0.f, 0.f, 0.f };
			constexpr static floaty3 RoofPosition = { 0.f, 2.f * HalfThickness + RoomHeight, 0.f };
			constexpr static floaty3 NorthWallPosition = { 0.f, 0.5f * RoomHeight, -(HalfThickness + RoomRadius) };
			constexpr static floaty3 EastWallPosition = { HalfThickness + RoomRadius, 0.5f * RoomHeight, 0.f };
			constexpr static floaty3 SouthWallPosition = { 0.f, 0.5f * RoomHeight, HalfThickness + RoomRadius };
			constexpr static floaty3 WestWallPosition = { -(HalfThickness + RoomRadius), 0.5f * RoomHeight, 0.f };

			constexpr static float TargetTime = 5.f;
			constexpr static float TargetY = -20.f;

			GameTimer ChangeTimer;
			floaty2 TargetPosition;
			floaty3 TargetOrientation;
			::G1I::CubeyShape *Floor = nullptr, *NorthWall = nullptr, *EastWall = nullptr, *SouthWall = nullptr, *WestWall = nullptr, *Roof = nullptr, *SecondFloor = nullptr;
			PuzzlePlayer *Player = nullptr;
			IStageManager *man = nullptr;
			bool Completed = false; // Puzzle Completed
			bool Finished = false; // Transition done, ready for next puzzle
		};

		struct SecondRoom : virtual G1::IShape, virtual FullResourceHolder
		{
			SecondRoom(G1::IGSpace *container, CommonResources *resources, PuzzlePlayer *player, IStageManager *man, FirstRoom *firstroom = nullptr);

			void BeforeDraw() override {}
			void AfterDraw() override;

			constexpr static Material FloorMaterial = { floaty4{0.1f, 0.1f, 0.1f, 1.f}, floaty4{0.f, 0.f, 0.f, 1.f}, floaty4{1.0f, 1.0f, 1.0f, 1.f}, floaty4{1.0f, 1.0f, 1.f, 25.f}, 1.f, 0 };
			constexpr static floaty3 FloorExtents = { 10.f, 0.2f, 10.f };
		protected:
			PuzzlePlayer *Player = nullptr;
			IStageManager *Manager = nullptr;
			::G1I::CubeyShape *Floor = nullptr;
		};

	}

}