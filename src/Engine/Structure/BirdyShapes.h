#pragma once

#include "BasicShapes.h"

namespace G1I
{

	constexpr int BIRDY_CONTROLLER_BITMASK = 0b0100000;
	constexpr int BIRDY_OBSTACLE_BITMASK = 0b10000000;
	constexpr int BIRDY_GROUND_BITMASK = 0b100000000;

	struct BirdyObstacle : G1::IShape
	{
		BirdyObstacle(G1::IGSpace *container, CommonResources *resources, float z, float heightgap, float y) : IShape(container, "Birdy Obstacle"), z(z) { Lower = new CubeyShape(container, resources, MyMaterial, floaty3{ 1.f, 2.f, 0.5f }, G1I::Static, nullptr, floaty3{ 0.f, y, z }); Upper = new CubeyShape(container, resources, MyMaterial, floaty3{ 1.f, 2.f, 0.5f }, G1I::Static, nullptr, floaty3{ 0.f, y + 4.f + heightgap, z }); this->AddChild(Lower); this->AddChild(Upper); }

		void BeforeDraw() override {}
		void AfterDraw() override {}

		void Update(float z, float heightgap, float y);
		float GetZ();

		constexpr static Material MyMaterial = { {0.1f, 0.1f, 0.1f, 1.f}, {0.f, 0.f, 0.f, 1.f}, {0.1f, 0.05f, 0.05f, 1.f}, {0.8f, 0.8f, 0.8f, 1.f}, 1.f, 0 };
	protected:
		float z{ 0.f };
		CubeyShape *Upper{ nullptr };
		CubeyShape *Lower{ nullptr };
	};

	struct IBirdyObstaclePositionIndicator
	{
		virtual float GetZPos() = 0;
	};

	struct BirdyObstacleInator : G1::IShape
	{
		BirdyObstacleInator(G1::IGSpace *container, CommonResources *resources, Pointer::f_ptr<IBirdyObstaclePositionIndicator> positionindicator);

		void BeforeDraw() override {}
		void AfterDraw() override;

	protected:
		float GetRandomHeight();
		float GetRandomGap();
		float GetRandomHeightGap();

		void UpdateObstacles(size_t count);

		constexpr static size_t ObstacleCount = 20;
		constexpr static float BaseHeight = 1.f;
		constexpr static float HeightDifference = 1.5f;
		constexpr static float PeriodDifference = 3.f;
		constexpr static float HeightGapLower = 2.f;
		constexpr static float HeightGapUpper = 5.f;
		constexpr static float BasePeriod = 2.5f; // Difference in z (distance between one obstacle and the next)
		constexpr static float ObstacleLingerLength = 25.f; // How far behind the camera obstacles will start to disappear

		Pointer::f_ptr<IBirdyObstaclePositionIndicator> PosInd;
		size_t CurrentFront{ 0ull };
		std::array<BirdyObstacle*, ObstacleCount> Obstacles;
	};

	struct BirdyController : PhysicsShape, Events::IEventListenerT<Events::AfterPhysicsEvent, Events::KeyEvent, Events::ResizeWindowEvent, Events::WindowFocusEvent, Events::MouseButtonEvent>, IBirdyObstaclePositionIndicator
	{
		BirdyController(G1::IGSpace *container, CommonResources *resources);

		void BeforeDraw() override;
		void AfterDraw() override;

		inline float GetZPos() override { return Cam->GetPosition().z; }

		// IEventListener/IAttachable methods
		bool Receive(Events::IEvent *event) override;
		bool Receive(Event::AfterPhysicsEvent *e) override;
		bool Receive(Event::KeyInput *e) override;
		bool Receive(Event::MouseButton *e) override;
		bool Receive(Event::WindowFocusEvent *e) override;

		bool Receive(Event::ResizeEvent *e) override;

		inline std::string GetName() const override { return Name; }
		
		static std::unique_ptr<btSphereShape> Shape;
		std::shared_ptr<btRigidBody> RigidBody;
		Pointer::selfish_ptr<Camera> Cam;

	public:
		constexpr static float SphereRadius = 0.5f;
		constexpr static float Mass = 5.f;
		constexpr static float Speed = 3.f;
		constexpr static float FastSpeed = 6.f;
		constexpr static float JumpPower = 25.f;
		constexpr static float RecoveryJumpPower = 0.08f;
		constexpr static float LungePower = 100.f;
		constexpr static float MaxUpSpeed = 10.f;

		bool Sprint = false;
		bool Jump = false;
		bool mouse_locked = false;
		bool outputcaminfo = true;
	};

	

	struct BirdyGround
	{

	};

}