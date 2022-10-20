#pragma once

#include "Drawing/Graphics1.h"
#include "Systems/Timer/Timer.h"

#include <BulletCollision/CollisionDispatch/btGhostObject.h>

namespace G1I
{
	enum PhysicsMasks
	{
		HitBoxes = 64,
	};

	struct PhysicsSpace : virtual G1::IGSpace
	{
	protected:
		struct internalphysicsevent_guy : btActionInterface
		{
			CommonResources *ass;

			void updateAction(btCollisionWorld *world, btScalar timestep) override;

#pragma warning(suppress:4100)
			void debugDraw(btIDebugDraw *draw) override {}
		};

		std::vector<std::shared_ptr<btRigidBody>> RigidBodies;
		std::vector<std::shared_ptr<btCollisionObject>> CollisionObjects;

		std::unique_ptr<btCollisionConfiguration> CollisionConfig;
		std::unique_ptr<btCollisionDispatcher> CollisionDispatcher;
		std::unique_ptr<btBroadphaseInterface> OverlappingPairCache;
		std::unique_ptr<btConstraintSolver> Solver;
		std::unique_ptr<btGhostPairCallback> GhostCallback;
		std::shared_ptr<btDynamicsWorld> DynamicsWorld;

		std::unordered_multimap<const btCollisionObject *, BulletThingo::IContactListener *> m_ContactListeners;

		virtual void AddCollisionObject(btCollisionObject * obj, int collisionFilterGroup = btBroadphaseProxy::StaticFilter, int collisionFilterMask = btBroadphaseProxy::AllFilter^btBroadphaseProxy::StaticFilter) override;
		virtual void AddRigidBody(btRigidBody * body) override;
		virtual void AddRigidBody(btRigidBody * body, int group, int mask) override;
		virtual void RemoveCollisionObject(btCollisionObject * obj) override;
		virtual void RemoveRigidBody(btRigidBody * body) override;
		virtual void SimulatePhysics(double interval) override;

		void DoContactListen();
	public:
		PhysicsSpace();
		virtual ~PhysicsSpace() {}
		
		void DoPhysics() override;

		inline std::weak_ptr<btDynamicsWorld> GetPhysicsWorld() override { return DynamicsWorld; }
		void RequestPhysicsCall(std::shared_ptr<btCollisionObject> obj, int collisionFilterGroup = btBroadphaseProxy::StaticFilter, int collisionFilterMask = btBroadphaseProxy::AllFilter^btBroadphaseProxy::StaticFilter) override;
		void RequestPhysicsCall(std::shared_ptr<btRigidBody> body, int collisionFilterGroup, int collisionFilterMask) override;
		void RequestPhysicsCall(std::shared_ptr<btRigidBody> body) override;
		void RequestPhysicsRemoval(btRigidBody *body) override;
		void RequestPhysicsRemoval(btCollisionObject *obj) override;

		void AddContactListener(BulletThingo::IContactListener *listener) override;
		void RemoveContactListener(BulletThingo::IContactListener *listener) override;

		Debug::DebugReturn Request(Requests::Request& Action) override;
	};
}