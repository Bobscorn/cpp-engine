#pragma once

#include "Helpers/DrawingHelper.h"

#include "Drawing/Graphics1.h"
#include "Drawing/Graphics3D.h"
#include "Drawing/IRen3D.h"
#include "Systems/Input/InputAttachment.h"
#include "Systems/Timer/Timer.h"

#include "Drawing/DrawCallReference.h"

#include <memory>

#ifdef USE_BULLET_STUFF
#pragma warning(push, 0)
#include <LinearMath/btMotionState.h>
#pragma warning(pop)
#endif

namespace G1I
{
	struct RenderShape : virtual G1::IShape
	{
		RenderShape(G1::IGSpace *container, std::vector<FullVertex> vertices, std::vector<unsigned int> indices, Material mat = Material(), Matrixy4x4 trans = Matrixy4x4::Identity());
		RenderShape(G1::IGSpace *container, std::vector<FullVertex> vertices, std::vector<unsigned int> indices, std::shared_ptr<Material> mat, Matrixy4x4 trans = Matrixy4x4::Identity());
		RenderShape(G1::IGSpace *container, Drawing::DrawCall call);
		RenderShape(G1::IGSpace *container, std::string filename);
		RenderShape(G1::IGSpace *container, CommonResources *resources, std::vector<FullVertex> vertices, std::vector<unsigned int> indices, Material mat = Material(), Matrixy4x4 trans = Matrixy4x4::Identity()) : RenderShape(container, vertices, indices, mat, trans) { mResources = resources; }
		virtual ~RenderShape() { mResources->Ren3->RemoveDrawCall(DrawCallKey); };

		void SetWorld(Matrixy4x4 trans);
		Matrixy4x4 GetWorld();

		[[nodiscard]] size_t GetDrawCallKey();

	protected:
		Matrixy4x4 World = Matrixy4x4::Identity();

		size_t DrawCallKey{ 0ull };
	};

	struct PureRenderShape : virtual G1::IShape, RenderShape
#ifdef POLYMORPHIC_SHAPE_CHILDREN
		, G1::SelfishVectorChildrenShape
#endif
	{
		PureRenderShape(G1::IGSpace *container, CommonResources *resources, std::vector<FullVertex> vertices, std::vector<unsigned int> indices, Material mat = Material()) : RenderShape(container, resources, vertices, indices, mat) {}

		void BeforeDraw() override {}
		void AfterDraw() override {}
	};

	struct PhysicsRenderShape : RenderShape, public btMotionState
	{
		PhysicsRenderShape(G1::IGSpace *container, std::vector<FullVertex> vertices, std::vector<unsigned int> indices, Material mat = Material(), Matrixy4x4 trans = Matrixy4x4::Identity()) : RenderShape(container, vertices, indices, mat, trans) {}
		PhysicsRenderShape(G1::IGSpace *container, std::vector<FullVertex> vertices, std::vector<unsigned int> indices, std::shared_ptr<Material> mat, Matrixy4x4 trans = Matrixy4x4::Identity()) : RenderShape(container, vertices, indices, mat, trans) {}
		PhysicsRenderShape(G1::IGSpace *container, Drawing::DrawCall call) : RenderShape(container, call) {}
		PhysicsRenderShape(G1::IGSpace *container, std::string filename) : RenderShape(container, filename) {}
		PhysicsRenderShape(G1::IGSpace *container, CommonResources *resources, std::vector<FullVertex> vertices, std::vector<unsigned int> indices, Material mat = Material(), Matrixy4x4 trans = Matrixy4x4::Identity()) : PhysicsRenderShape(container, vertices, indices, mat, trans) { mResources = resources; }
		PhysicsRenderShape(G1::IGSpace *container, CommonResources *resources, std::vector<FullVertex> vertices, std::vector<unsigned int> indices, std::shared_ptr<Material> mat, Matrixy4x4 trans = Matrixy4x4::Identity()) : PhysicsRenderShape(container, vertices, indices, mat, trans) { mResources = resources; }
		virtual ~PhysicsRenderShape() {};

		void getWorldTransform(btTransform& t) const override;
		void setWorldTransform(const btTransform& t) override;

		floaty3 GetPosition();

		virtual void SetUserPointer(void *p) = 0;
		virtual void SetUserIndex(int i) = 0;
		virtual void SetUserIndex2(int i) = 0;
	};

	struct PhysicsShape : btMotionState, virtual G1::IShape
	{
		PhysicsShape(floaty3 pos = { 0.f, 0.f, 0.f }) : World(Matrixy4x4::Translate(pos.x, pos.y, pos.z)) {}
		PhysicsShape(CommonResources* resources, floaty3 pos = { 0.f, 0.f, 0.f }) : World(Matrixy4x4::Translate(pos.x, pos.y, pos.z)) { (void)resources; }
		PhysicsShape(Matrixy4x4 trans) : World(trans) {}

		virtual ~PhysicsShape() {}
	
		void getWorldTransform(btTransform& t) const override;
		void setWorldTransform(const btTransform& t) override;

		floaty3 GetPosition();
	protected:
		Matrixy4x4 World;
	};
	
	constexpr std::array<FullVertex, 24> CubeVertices = {
			FullVertex{ floaty3(+0.500000f, +0.500000f, +0.500000f), floaty3(-0.000000f, -1.000000f, +0.000000f), floaty3(-1.000000f, +0.000000f, +0.000000f), floaty3(+0.000000f, +0.000000f, +1.000000f), floaty2(+0.000000f, +1.000000f) },
			FullVertex{ floaty3(-0.500000f, -0.500000f, +0.500000f), floaty3(-0.000000f, -1.000000f, +0.000000f), floaty3(-1.000000f, +0.000000f, +0.000000f), floaty3(+0.000000f, +0.000000f, +1.000000f), floaty2(+1.000000f, +0.000000f) },
			FullVertex{ floaty3(-0.500000f, +0.500000f, +0.500000f), floaty3(-0.000000f, -1.000000f, +0.000000f), floaty3(-1.000000f, +0.000000f, +0.000000f), floaty3(+0.000000f, +0.000000f, +1.000000f), floaty2(+0.000000f, +0.000000f) },
			FullVertex{ floaty3(-0.500000f, +0.500000f, -0.500000f), floaty3(-1.000000f, +0.000000f, +0.000000f), floaty3(-0.000000f, -1.000000f, +0.000000f), floaty3(+0.000000f, +0.000000f, -1.000000f), floaty2(+1.000000f, +1.000000f) },
			FullVertex{ floaty3(+0.500000f, -0.500001f, -0.500000f), floaty3(-1.000000f, +0.000000f, +0.000000f), floaty3(-0.000000f, -1.000000f, +0.000000f), floaty3(+0.000000f, +0.000000f, -1.000000f), floaty2(+0.000000f, +0.000000f) },
			FullVertex{ floaty3(+0.500000f, +0.500000f, -0.500000f), floaty3(-1.000000f, +0.000000f, +0.000000f), floaty3(-0.000000f, -1.000000f, +0.000000f), floaty3(+0.000000f, +0.000000f, -1.000000f), floaty2(+0.000000f, +1.000000f) },
			FullVertex{ floaty3(+0.500000f, +0.500000f, -0.500000f), floaty3(+0.000000f, -0.000000f, -1.000000f), floaty3(-0.000000f, -1.000000f, +0.000000f), floaty3(+1.000000f, +0.000000f, +0.000000f), floaty2(+1.000000f, +1.000000f) },
			FullVertex{ floaty3(+0.500000f, -0.500000f, +0.500000f), floaty3(+0.000000f, -0.000000f, -1.000000f), floaty3(-0.000000f, -1.000000f, +0.000000f), floaty3(+1.000000f, +0.000000f, +0.000000f), floaty2(+0.000000f, +0.000000f) },
			FullVertex{ floaty3(+0.500000f, +0.500000f, +0.500000f), floaty3(+0.000000f, -0.000000f, -1.000000f), floaty3(+0.000000f, -1.000000f, +0.000000f), floaty3(+1.000000f, +0.000000f, +0.000000f), floaty2(+0.000000f, +1.000000f) },
			FullVertex{ floaty3(+0.500000f, -0.500001f, -0.500000f), floaty3(-0.000000f, -0.000000f, -1.000000f), floaty3(-1.000000f, +0.000000f, +0.000000f), floaty3(+0.000000f, -1.000000f, +0.000000f), floaty2(+1.000000f, +1.000000f) },
			FullVertex{ floaty3(-0.500000f, -0.500000f, +0.500000f), floaty3(-0.000000f, -0.000000f, -1.000000f), floaty3(-1.000000f, +0.000000f, +0.000000f), floaty3(+0.000000f, -1.000000f, +0.000000f), floaty2(+0.000000f, +0.000000f) },
			FullVertex{ floaty3(+0.500000f, -0.500000f, +0.500000f), floaty3(-0.000000f, -0.000000f, -1.000000f), floaty3(-1.000000f, +0.000000f, +0.000000f), floaty3(+0.000000f, -1.000000f, +0.000000f), floaty2(+0.000000f, +1.000000f) },
			FullVertex{ floaty3(-0.500000f, -0.500000f, +0.500000f), floaty3(-0.000000f, +0.000000f, -1.000000f), floaty3(+0.000000f, +1.000000f, -0.000000f), floaty3(-1.000000f, +0.000000f, +0.000000f), floaty2(+0.000000f, +1.000000f) },
			FullVertex{ floaty3(-0.500000f, +0.500000f, -0.500000f), floaty3(-0.000000f, +0.000000f, -1.000000f), floaty3(+0.000000f, +1.000000f, -0.000000f), floaty3(-1.000000f, +0.000000f, +0.000000f), floaty2(+1.000000f, +0.000000f) },
			FullVertex{ floaty3(-0.500000f, +0.500000f, +0.500000f), floaty3(-0.000000f, +0.000000f, -1.000000f), floaty3(+0.000000f, +1.000000f, -0.000000f), floaty3(-1.000000f, +0.000000f, +0.000000f), floaty2(+0.000000f, +0.000000f) },
			FullVertex{ floaty3(+0.500000f, +0.500000f, +0.500000f), floaty3(+0.000000f, +0.000000f, +1.000000f), floaty3(-1.000000f, +0.000000f, -0.000000f), floaty3(+0.000000f, +1.000000f, -0.000000f), floaty2(+1.000000f, +1.000000f) },
			FullVertex{ floaty3(-0.500000f, +0.500000f, -0.500000f), floaty3(+0.000000f, +0.000000f, +1.000000f), floaty3(-1.000000f, +0.000000f, -0.000000f), floaty3(+0.000000f, +1.000000f, -0.000000f), floaty2(+0.000000f, +0.000000f) },
			FullVertex{ floaty3(+0.500000f, +0.500000f, -0.500000f), floaty3(+0.000000f, +0.000000f, +1.000000f), floaty3(-1.000000f, +0.000000f, -0.000000f), floaty3(+0.000000f, +1.000000f, -0.000000f), floaty2(+0.000000f, +1.000000f) },
			FullVertex{ floaty3(+0.500000f, -0.500000f, +0.500000f), floaty3(+0.000000f, -1.000000f, +0.000000f), floaty3(-1.000000f, +0.000000f, +0.000000f), floaty3(+0.000000f, +0.000000f, +1.000000f), floaty2(+1.000000f, +1.000000f) },
			FullVertex{ floaty3(-0.500000f, -0.500000f, -0.500000f), floaty3(-1.000000f, +0.000001f, +0.000000f), floaty3(+0.000000f, -1.000000f, +0.000000f), floaty3(+0.000000f, +0.000000f, -1.000000f), floaty2(+1.000000f, +0.000000f) },
			FullVertex{ floaty3(+0.500000f, -0.500001f, -0.500000f), floaty3(-0.000000f, -0.000000f, -1.000000f), floaty3(-0.000001f, -1.000000f, +0.000000f), floaty3(+1.000000f, -0.000001f, -0.000000f), floaty2(+1.000000f, +0.000000f) },
			FullVertex{ floaty3(-0.500000f, -0.500000f, -0.500000f), floaty3(-0.000000f, +0.000000f, -1.000000f), floaty3(-1.000000f, +0.000000f, +0.000000f), floaty3(-0.000000f, -1.000000f, -0.000000f), floaty2(+1.000000f, +0.000000f) },
			FullVertex{ floaty3(-0.500000f, -0.500000f, -0.500000f), floaty3(-0.000000f, +0.000000f, -1.000000f), floaty3(+0.000000f, +1.000000f, -0.000000f), floaty3(-1.000000f, +0.000000f, +0.000000f), floaty2(+1.000000f, +1.000000f) },
			FullVertex{ floaty3(-0.500000f, +0.500000f, +0.500000f), floaty3(+0.000000f, -0.000000f, +1.000000f), floaty3(-1.000000f, +0.000000f, +0.000000f), floaty3(+0.000000f, +1.000000f, -0.000000f), floaty2(+1.000000f, +0.000000f) }
	};
	constexpr std::array<unsigned int, 36> CubeIndices = { 2, 1, 0, 5, 4, 3, 8, 7, 6, 11, 10, 9, 14, 13, 12, 17, 16, 15, 1, 18, 0, 4, 19, 3, 7, 20, 6, 10, 21, 9, 13, 22, 12, 16, 23, 15, };

	struct CheapCubeShape : PhysicsRenderShape
	{
		CheapCubeShape(G1::IGSpace *container, CommonResources *resources, Material mat = Material(), Matrixy4x4 trans = Matrixy4x4::Identity(), bool dynamic = true);
		CheapCubeShape(G1::IGSpace *container, CommonResources *resources, std::string name, floaty3 Position, Material mat = Material(), bool dynamic = true) : CheapCubeShape(container, resources, mat, Matrixy4x4::Translate(Position), dynamic) { Name = name; }

		bool Dynamic;
		static std::unique_ptr<btCollisionShape> bShape;
		std::shared_ptr<btRigidBody> RigidBody;

		void BeforeDraw() override {}
		void AfterDraw() override {}

		void SetUserPointer(void *p) override { RigidBody->setUserPointer(p); }
		void SetUserIndex(int i) override { RigidBody->setUserIndex(i); }
		void SetUserIndex2(int i) override { RigidBody->setUserIndex2(i); }
	};

	struct CubeShape : PhysicsShape, virtual G1::IShape
	{
		CubeShape(G1::IGSpace *container, CommonResources *resources, floaty3 extents = { 0.5f, 0.5f, 0.5f }, floaty3 pos = { 0.f, 0.f, 0.f }, bool dynamic = true);

		void BeforeDraw() override {}
		void AfterDraw() override {}
	protected:
		
		floaty3 Extents{ 0.5f, 0.5f, 0.5f };
		std::unique_ptr<btCollisionShape> bShape;
		std::shared_ptr<btRigidBody> RigidBody;
		bool Dynamic = true;
	};

	struct PhysicsThing
	{
		Matrixy4x4 Trans;
		bool Dynamic;
		float Mass;
		std::shared_ptr<btCollisionShape> Shape;
		float Friction = 0.5f;
		int CollisionGroup = 1;
		int CollisionMask = -1;
	};

	struct PhysicsBody : virtual FullResourceHolder, virtual G1::IShape, PhysicsShape
	{
		PhysicsBody(G1::IShapeThings things, PhysicsThing physthings);

		inline virtual void BeforeDraw() override {}
		inline virtual void AfterDraw() override {}

		std::shared_ptr<btCollisionShape> GetShape();
		inline std::shared_ptr<btRigidBody> GetBody() { return Body; }
	protected:
		PhysicsThing Info;
		std::shared_ptr<btRigidBody> Body;
	};

	struct BvhThings
	{
		std::vector<floaty3> Vertices;
		std::vector<int> Indices;
	};

	struct BvhPhysicsBody : PhysicsShape
	{
		BvhPhysicsBody(G1::IShapeThings tings, PhysicsThing ptings, BvhThings btings);

		inline virtual void BeforeDraw() override {}
		inline virtual void AfterDraw() override {}

		std::shared_ptr<btTriangleIndexVertexArray> GetTriangleIndexVertexArray();

	protected:
		BvhThings m_BvhInfo;
		PhysicsThing m_PhysicsInfo;
		std::shared_ptr<btTriangleIndexVertexArray> m_MeshData;
		std::shared_ptr<btBvhTriangleMeshShape> m_BvhObject;
		std::shared_ptr<btRigidBody> m_Body;
	};

	struct CubePRShape : PhysicsRenderShape, virtual G1::IShape
	{
		CubePRShape(G1::IGSpace *container, CommonResources *resources, std::string geodatafile, floaty3 extents = { 0.5f, 0.5f, 0.5f }, bool dynamic = true, float mass = 1.f);

		void BeforeDraw() override {}
		void AfterDraw() override {}

		void SetUserPointer(void *p) override { RigidBody->setUserPointer(p); }
		void SetUserIndex(int i) override { RigidBody->setUserIndex(i); }
		void SetUserIndex2(int i) override { RigidBody->setUserIndex2(i); }

	protected:
		floaty3 Extents{ 0.5f, 0.5f, 0.5f };
		std::unique_ptr<btCollisionShape> bShape;
		std::shared_ptr<btRigidBody> RigidBody;
		bool Dynamic = true;
	};

	std::vector<FullVertex> TransformVertices(floaty3 extents);

	enum bodytype
	{
		Static = 0b0001,
		Dynamic = 0b0010,
		Kinematic = 0b0100,
		Removed_Static = 0b1000,
		Removed_Dynamic = 0b00010000,
		Removed_Kinematic = 0b00100000
	};

	struct CubeyShape : PhysicsRenderShape, virtual G1::IShape
	{
		CubeyShape(G1::IGSpace *container, CommonResources *resources, Material mat = Material(), floaty3 extents = { 0.5f, 0.5f, 0.5f }, bodytype dynamic = Dynamic, btCollisionShape *shape = nullptr, floaty3 pos = { 0.f, 0.f, 0.f }, int CollisionGroup = btBroadphaseProxy::StaticFilter, int CollisionMask = btBroadphaseProxy::AllFilter ^ btBroadphaseProxy::StaticFilter);
		CubeyShape(G1::IGSpace *container, CommonResources *resources, Material mat = Material(), floaty3 extents = { 0.5f, 0.5f, 0.5f }, bodytype dynamic = Dynamic, btCollisionShape *shape = nullptr, Matrixy4x4 = Matrixy4x4::Identity(), int CollisionGroup = btBroadphaseProxy::StaticFilter, int CollisionMask = btBroadphaseProxy::AllFilter ^ btBroadphaseProxy::StaticFilter);
		CubeyShape(G1::IGSpace *container, CommonResources *resources, std::shared_ptr<Material> mat, floaty3 extents = { 0.5f, 0.5f, 0.5f }, bodytype dynamic = Dynamic, btCollisionShape *shape = nullptr, Matrixy4x4 = Matrixy4x4::Identity(), int CollisionGroup = btBroadphaseProxy::StaticFilter, int CollisionMask = btBroadphaseProxy::AllFilter ^ btBroadphaseProxy::StaticFilter);
		CubeyShape(G1::IGSpace *container, CommonResources *resources, Drawing::DrawCall call, bodytype dynamic, btCollisionShape *shape, Matrixy4x4 trans = Matrixy4x4::Identity(), int CollisionGroup = btBroadphaseProxy::StaticFilter, int CollisionMask = btBroadphaseProxy::AllFilter ^ btBroadphaseProxy::StaticFilter);
		~CubeyShape();

		virtual void BeforeDraw() override {}
		virtual void AfterDraw() override {}

		inline void EnableDeactivation() { RigidBody->forceActivationState(WANTS_DEACTIVATION); }
		inline void DisableDeactivation() { RigidBody->setActivationState(DISABLE_DEACTIVATION); }

		void SetPosition(floaty3 pos);
		void SetRotation(floaty3 rot);
		void Disable();
		void Enable(int collisiongroup = btBroadphaseProxy::DefaultFilter, int collisionmask = btBroadphaseProxy::AllFilter);
		bool Enabled();

		[[nodiscard]] inline btRigidBody *GetBody() { return RigidBody.get(); }
		[[nodiscard]] inline int GetType() { return BodyType; }

		virtual void SetUserPointer(void * p) override;
		virtual void SetUserIndex(int i) override;
		virtual void SetUserIndex2(int i) override;
	protected:

		floaty3 Extents{ 0.5f, 0.5f, 0.5f };
		std::unique_ptr<btCollisionShape> bShape;
		std::shared_ptr<btRigidBody> RigidBody;
		int BodyType = Dynamic;

	};

	// Contains no World Matrix, Material, Textures or anything
	// Only a DrawCall
	struct RawRendyShape : G1::IShape
	{
		RawRendyShape(G1::IShapeThings things, Drawing::DrawCall call);
		virtual ~RawRendyShape();

		inline virtual void BeforeDraw() override {}
		inline virtual void AfterDraw() override {}

		Drawing::DrawCall *GetCall();

	protected:
		size_t key;
	};

	struct RawMatrixRendyShape : RawRendyShape
	{
		RawMatrixRendyShape(G1::IShapeThings things, Drawing::DrawCall, Matrixy4x4 mat);
		virtual ~RawMatrixRendyShape() {}

	protected:
		Matrixy4x4 World;
	};
	
	struct NoFreeLightException : std::exception
	{
		inline virtual const char *what() const noexcept override { return "No Free lights available"; }
	};

	struct LightShape : G1::IShape
	{
		LightShape(G1::IShapeThings ting, Light light);
		~LightShape();

		virtual void BeforeDraw() override;
		virtual void AfterDraw() override;

		Light *GetLight();

	protected:
		static size_t GetFreeIndex(); // Throws NoFreeLightException
		size_t m_LightIndex = (size_t)-1;

		static std::array<bool, LIGHT_COUNT> TakenLights;
	public:
		static size_t GetNumUsedLights();
	};
	
	namespace EnviroThings
	{
		constexpr float scale = 15.f;
		constexpr std::array<unsigned int, 36> Indices = { 2, 1, 0, 5, 4, 3, 8, 7, 6, 11, 10, 9, 14, 13, 12, 17, 16, 15, 1, 18, 0, 4, 19, 3, 7, 20, 6, 10, 21, 9, 13, 22, 12, 16, 23, 15, };
		constexpr std::array<FullVertex, 24> Vertices = {
			FullVertex({ floaty3(+5.000000f * scale, +0.100000f, +5.000000f * scale), floaty3(-0.000000f, -1.000000f, +0.000000f), floaty3(-1.000000f, +0.000000f, +0.000000f), floaty3(+0.000000f, +0.000000f, +1.000000f), floaty2(+0.000000f, +1.000000f) }),
			FullVertex({ floaty3(-5.000000f * scale, -0.100000f, +5.000000f * scale), floaty3(-0.000000f, -1.000000f, +0.000000f), floaty3(-1.000000f, +0.000000f, +0.000000f), floaty3(+0.000000f, +0.000000f, +1.000000f), floaty2(+1.000000f, +0.000000f) }),
			FullVertex({ floaty3(-5.000000f * scale, +0.100000f, +5.000000f * scale), floaty3(-0.000000f, -1.000000f, +0.000000f), floaty3(-1.000000f, +0.000000f, +0.000000f), floaty3(+0.000000f, +0.000000f, +1.000000f), floaty2(+0.000000f, +0.000000f) }),
			FullVertex({ floaty3(-5.000000f * scale, +0.100000f, -5.000000f * scale), floaty3(-1.000000f, +0.000000f, +0.000000f), floaty3(-0.000000f, -1.000000f, +0.000000f), floaty3(+0.000000f, +0.000000f, -1.000000f), floaty2(+1.000000f, +1.000000f) }),
			FullVertex({ floaty3(+4.999999f * scale, -0.100001f, -5.000000f * scale), floaty3(-1.000000f, +0.000000f, +0.000000f), floaty3(-0.000000f, -1.000000f, +0.000000f), floaty3(+0.000000f, +0.000000f, -1.000000f), floaty2(+0.000000f, +0.000000f) }),
			FullVertex({ floaty3(+5.000000f * scale, +0.100000f, -5.000000f * scale), floaty3(-1.000000f, +0.000000f, +0.000000f), floaty3(-0.000000f, -1.000000f, +0.000000f), floaty3(+0.000000f, +0.000000f, -1.000000f), floaty2(+0.000000f, +1.000000f) }),
			FullVertex({ floaty3(+5.000000f * scale, +0.100000f, -5.000000f * scale), floaty3(+0.000000f, -0.000000f, -1.000000f), floaty3(-0.000000f, -1.000000f, +0.000000f), floaty3(+1.000000f, +0.000000f, +0.000000f), floaty2(+1.000000f, +1.000000f) }),
			FullVertex({ floaty3(+5.000000f * scale, -0.100000f, +5.000000f * scale), floaty3(+0.000000f, -0.000000f, -1.000000f), floaty3(-0.000000f, -1.000000f, +0.000000f), floaty3(+1.000000f, +0.000000f, +0.000000f), floaty2(+0.000000f, +0.000000f) }),
			FullVertex({ floaty3(+5.000000f * scale, +0.100000f, +5.000000f * scale), floaty3(+0.000000f, -0.000000f, -1.000000f), floaty3(+0.000000f, -1.000000f, +0.000000f), floaty3(+1.000000f, +0.000000f, +0.000000f), floaty2(+0.000000f, +1.000000f) }),
			FullVertex({ floaty3(+4.999999f * scale, -0.100001f, -5.000000f * scale), floaty3(-0.000000f, -0.000000f, -1.000000f), floaty3(-1.000000f, +0.000000f, +0.000000f), floaty3(+0.000000f, -1.000000f, +0.000000f), floaty2(+1.000000f, +1.000000f) }),
			FullVertex({ floaty3(-5.000000f * scale, -0.100000f, +5.000000f * scale), floaty3(-0.000000f, -0.000000f, -1.000000f), floaty3(-1.000000f, +0.000000f, +0.000000f), floaty3(+0.000000f, -1.000000f, +0.000000f), floaty2(+0.000000f, +0.000000f) }),
			FullVertex({ floaty3(+5.000000f * scale, -0.100000f, +5.000000f * scale), floaty3(-0.000000f, -0.000000f, -1.000000f), floaty3(-1.000000f, +0.000000f, +0.000000f), floaty3(+0.000000f, -1.000000f, +0.000000f), floaty2(+0.000000f, +1.000000f) }),
			FullVertex({ floaty3(-5.000000f * scale, -0.100000f, +5.000000f * scale), floaty3(-0.000000f, +0.000000f, -1.000000f), floaty3(+0.000000f, +1.000000f, -0.000000f), floaty3(-1.000000f, +0.000000f, +0.000000f), floaty2(+0.000000f, +1.000000f) }),
			FullVertex({ floaty3(-5.000000f * scale, +0.100000f, -5.000000f * scale), floaty3(-0.000000f, +0.000000f, -1.000000f), floaty3(+0.000000f, +1.000000f, -0.000000f), floaty3(-1.000000f, +0.000000f, +0.000000f), floaty2(+1.000000f, +0.000000f) }),
			FullVertex({ floaty3(-5.000000f * scale, +0.100000f, +5.000000f * scale), floaty3(-0.000000f, +0.000000f, -1.000000f), floaty3(+0.000000f, +1.000000f, -0.000000f), floaty3(-1.000000f, +0.000000f, +0.000000f), floaty2(+0.000000f, +0.000000f) }),
			FullVertex({ floaty3(+5.000000f * scale, +0.100000f, +5.000000f * scale), floaty3(+0.000000f, +0.000000f, +1.000000f), floaty3(-1.000000f, +0.000000f, -0.000000f), floaty3(+0.000000f, +1.000000f, -0.000000f), floaty2(+1.000000f, +1.000000f) }),
			FullVertex({ floaty3(-5.000000f * scale, +0.100000f, -5.000000f * scale), floaty3(+0.000000f, +0.000000f, +1.000000f), floaty3(-1.000000f, +0.000000f, -0.000000f), floaty3(+0.000000f, +1.000000f, -0.000000f), floaty2(+0.000000f, +0.000000f) }),
			FullVertex({ floaty3(+5.000000f * scale, +0.100000f, -5.000000f * scale), floaty3(+0.000000f, +0.000000f, +1.000000f), floaty3(-1.000000f, +0.000000f, -0.000000f), floaty3(+0.000000f, +1.000000f, -0.000000f), floaty2(+0.000000f, +1.000000f) }),
			FullVertex({ floaty3(+5.000000f * scale, -0.100000f, +5.000000f * scale), floaty3(+0.000000f, -1.000000f, +0.000000f), floaty3(-1.000000f, +0.000000f, +0.000000f), floaty3(+0.000000f, +0.000000f, +1.000000f), floaty2(+1.000000f, +1.000000f) }),
			FullVertex({ floaty3(-5.000000f * scale, -0.100000f, -5.000000f * scale), floaty3(-1.000000f, +0.000001f, +0.000000f), floaty3(+0.000000f, -1.000000f, +0.000000f), floaty3(+0.000000f, +0.000000f, -1.000000f), floaty2(+1.000000f, +0.000000f) }),
			FullVertex({ floaty3(+4.999999f * scale, -0.100001f, -5.000000f * scale), floaty3(-0.000000f, -0.000000f, -1.000000f), floaty3(-0.000001f, -1.000000f, +0.000000f), floaty3(+1.000000f, -0.000001f, -0.000000f), floaty2(+1.000000f, +0.000000f) }),
			FullVertex({ floaty3(-5.000000f * scale, -0.100000f, -5.000000f * scale), floaty3(-0.000000f, +0.000000f, -1.000000f), floaty3(-1.000000f, +0.000000f, +0.000000f), floaty3(-0.000000f, -1.000000f, -0.000000f), floaty2(+1.000000f, +0.000000f) }),
			FullVertex({ floaty3(-5.000000f * scale, -0.100000f, -5.000000f * scale), floaty3(-0.000000f, +0.000000f, -1.000000f), floaty3(+0.000000f, +1.000000f, -0.000000f), floaty3(-1.000000f, +0.000000f, +0.000000f), floaty2(+1.000000f, +1.000000f) }),
			FullVertex({ floaty3(-5.000000f * scale, +0.1000000f, +5.000000f * scale), floaty3(+0.000000f, -0.000000f, +1.000000f), floaty3(-1.000000f, +0.000000f, +0.000000f), floaty3(+0.000000f, +1.000000f, -0.000000f), floaty2(+1.000000f, +0.000000f) })
		};
		constexpr Material Mat(floaty4(0.5f, 0.f, 0.f, 1.f), floaty4(0.f, 0.f, 0.f, 1.f), floaty4(0.7f, 0.5f, 0.5f, 1.f), floaty4(0.8f, 0.8f, 0.8f, 1.f), 1.f, 0u);
		constexpr floaty3 Pos = { 0.f, -5.f, 0.f };
	}

	struct EnviroShape : PhysicsRenderShape
	{
		EnviroShape(G1::IGSpace *container, CommonResources *resources, std::string name = "Enviro");

		void BeforeDraw() override {};
		void AfterDraw() override {};

		void SetUserPointer(void *p) override { RigidBody->setUserPointer(p); }
		void SetUserIndex(int i) override { RigidBody->setUserIndex(i); }
		void SetUserIndex2(int i) override { RigidBody->setUserIndex2(i); }

		static std::unique_ptr<btCollisionShape> Shape;
		std::shared_ptr<btRigidBody> RigidBody;
	};

	struct NormalCamera : G1::IShape, Events::IEventListener
	{
		NormalCamera(G1::IGSpace *container, CommonResources *resources);

		virtual void BeforeDraw() override;
		virtual void AfterDraw() override;

		bool Receive(Events::IEvent *event) override;

		float multiplier = 1.0f;
		float dx = 0.0, dy = 0.0;
		bool mouse_locked = false;
		Pointer::selfish_ptr<Camera> Cam;
	};

	struct DynamicCharacterController : PhysicsShape, InputAttach::IAttachable, Events::IEventListenerT<Events::WindowFocusEvent, Events::ResizeWindowEvent, Events::AfterPhysicsEvent, Events::MouseButtonEvent, Events::KeyEvent>
	{
		DynamicCharacterController(G1::IGSpace *container, CommonResources *resources);

		void BeforeDraw() override;
		void AfterDraw() override;

		// IEventListener/IAttachable methods
		bool Receive(Events::IEvent *event) override;
		bool Receive(Event::KeyInput *e) override;
		bool Receive(Event::MouseMove *e) override;
		bool Receive(Event::MouseButton *e) override;
		bool Receive(Event::MouseWheelButton *e) override;
		bool Receive(Event::MouseWheel *e) override;

		bool Receive(Event::ResizeEvent *e) override;

		// IAttachable methods
		inline void Superseded() override
		{
			mouse_locked = false;
		}
		inline void Reinstated() override
		{
			mouse_locked = true;
		}
		inline std::string GetName() const override { return Name; }

		// Call once per frame, checks if rigidbody is on the ground, saves value to this->groundy
		bool OnGround(btDynamicsWorld *world);

		static std::unique_ptr<btCapsuleShape> Shape;
		std::shared_ptr<btRigidBody> RigidBody;
		Pointer::selfish_ptr<Camera> Cam;
		float multiplier = 1.f;
		bool mouse_locked = false;

		constexpr static btScalar walkpower = btScalar(1.5f);
		constexpr static btScalar airpower = btScalar(150.f);
		float forwardness = 0.f, rightness = 0.f;
		btScalar debugDot = 0.f;
	public:
		GameTimer JumpTimer;
		constexpr static float JumpCooldown = 0.25f;
		constexpr static float CapsuleHeight = 2.f;
		constexpr static float CapsuleRadius = 0.5f;
		bool forward = false, back = false, left = false, right = false;

		bool jump = false;
		bool groundy = false;
		bool outputcaminfo = true;
	};
#pragma warning(default:4250)

	struct ProfilerThings
	{
		double ProfileTime = 10.0;
		bool Running = false;
	};
	struct ProfilerShape : G1::IShape, Requests::IRequestable
	{
		ProfilerShape(G1::IShapeThings ting1, ProfilerThings ting2);

		virtual void BeforeDraw() override;
		virtual void AfterDraw() override;

		void Restart();

		virtual Debug::DebugReturn Request(Requests::Request &req) override;

		inline virtual std::string GetName() const override { return "Profiler McGee"; }
	protected:
		void ReportProfiling();
		void ReportProfiling(double time);

		GameTimer m_Timer;

		std::vector<double> m_DeltaTimes;
		size_t m_StartID = 0ull;
		ProfilerThings m_Info;
	};

	struct GLRen2TestShape : G1::IShape
	{
		GLRen2TestShape(G1::IShapeThings tings);

		inline virtual void BeforeDraw() override {}
		inline virtual void AfterDraw() override {}

	protected:
		Drawing::DrawCallReference m_DrawCallKey;
	};
}