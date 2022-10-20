//#include "Thing.h"
#include "PhysicsSpace.h"

#include "Helpers/ProfileHelper.h"

#include "Systems/Events/PhysicsEvent.h"
#include "Systems/Events/Events.h"
#include "Systems/Time/Time.h"

void G1I::PhysicsSpace::DoPhysics()
{
	SimulatePhysics((double)mResources->Time->GetFixedDeltaTime());
	DoContactListen();
}

void G1I::PhysicsSpace::AddCollisionObject(btCollisionObject * obj, int collisionFilterGroup, int collisionFilterMask)
{
	DynamicsWorld->addCollisionObject(obj, collisionFilterGroup, collisionFilterMask);
}

void G1I::PhysicsSpace::AddRigidBody(btRigidBody * body)
{
	DynamicsWorld->addRigidBody(body);
}

void G1I::PhysicsSpace::AddRigidBody(btRigidBody * body, int group, int mask)
{
	DynamicsWorld->addRigidBody(body, group, mask);
}

void G1I::PhysicsSpace::RemoveCollisionObject(btCollisionObject * obj)
{
	DynamicsWorld->removeCollisionObject(obj);
}

void G1I::PhysicsSpace::RemoveRigidBody(btRigidBody * body)
{
	DynamicsWorld->removeRigidBody(body);
}

void G1I::PhysicsSpace::SimulatePhysics(double interval)
{
	PROFILE_PUSH("Physics Simulation");

	if (interval != 0.0)
		DynamicsWorld->stepSimulation(btScalar(interval), 1);

	PROFILE_POP();
}

void G1I::PhysicsSpace::DoContactListen()
{
	int manifoldCount = DynamicsWorld->getDispatcher()->getNumManifolds();
	
	for (int i = 0; i < manifoldCount; ++i)
	{
		bool do_A, do_B;
		auto manifold = DynamicsWorld->getDispatcher()->getManifoldByIndexInternal(i);
		auto object_A = manifold->getBody0();
		auto object_B = manifold->getBody1();

		auto a_pair = m_ContactListeners.equal_range(object_A);
		do_A = a_pair.first != a_pair.second;

		auto b_pair = m_ContactListeners.equal_range(object_B);
		do_B = b_pair.first != b_pair.second;

		if (!do_A && !do_B)
			continue;

		int num_contacts = manifold->getNumContacts();
		for (int j = 0; j < num_contacts; ++j)
		{
			auto point = manifold->getContactPoint(j);
			if (do_A)
				for (auto it = a_pair.first; it != a_pair.second; ++it)
					it->second->OnContact(point);
			if (do_B)
				for (auto it = b_pair.first; it != b_pair.second; ++it)
					it->second->OnContact(point);
		}
	}
}

G1I::PhysicsSpace::PhysicsSpace()
{
	CollisionConfig = std::make_unique<btDefaultCollisionConfiguration>();
	CollisionDispatcher = std::make_unique<btCollisionDispatcher>(CollisionConfig.get());
	OverlappingPairCache = std::make_unique<btDbvtBroadphase>();
	Solver = std::make_unique<btSequentialImpulseConstraintSolver>();
	GhostCallback = std::make_unique<btGhostPairCallback>();
	DynamicsWorld = std::make_shared<btDiscreteDynamicsWorld>(CollisionDispatcher.get(), OverlappingPairCache.get(), Solver.get(), CollisionConfig.get());

	DynamicsWorld->setGravity({ 0.f, -9.8f, 0.f });
	DynamicsWorld->getBroadphase()->getOverlappingPairCache()->setInternalGhostPairCallback(GhostCallback.get());
}

void G1I::PhysicsSpace::RequestPhysicsCall(std::shared_ptr<btCollisionObject> body, int collisionFilterGroup, int collisionFilterMask)
{
	CollisionObjects.emplace_back(body);
	AddCollisionObject(body.get(), collisionFilterGroup, collisionFilterMask);
}

void G1I::PhysicsSpace::RequestPhysicsCall(std::shared_ptr<btRigidBody> body, int collisionFilterGroup, int collisionFilterMask)
{
	RigidBodies.emplace_back(body);
	AddRigidBody(body.get(), collisionFilterGroup, collisionFilterMask);
}

void G1I::PhysicsSpace::RequestPhysicsCall(std::shared_ptr<btRigidBody> body)
{
	RigidBodies.emplace_back(body);
	AddRigidBody(body.get());
}

void G1I::PhysicsSpace::RequestPhysicsRemoval(btRigidBody * body)
{
	for (size_t i = RigidBodies.size(); i-- > 0; )
	{
		auto &bod = RigidBodies[i];
		if (bod.get() == body)
		{
			RemoveRigidBody(body);
			std::swap(bod, RigidBodies.back());
			RigidBodies.pop_back();
			return;
		}
	}
	DWARNING("Could not find Rigid Body to remove");
}

void G1I::PhysicsSpace::RequestPhysicsRemoval(btCollisionObject * obj)
{
	for (size_t i = CollisionObjects.size(); i-- > 0; )
	{
		auto &cob = CollisionObjects[i];
		if (cob.get() == obj)
		{
			RemoveCollisionObject(obj);
			std::swap(cob, CollisionObjects.back());
			CollisionObjects.pop_back();
			return;
		}
	}
	DWARNING("Could not find Collision Object to remove");
}

void G1I::PhysicsSpace::AddContactListener(BulletThingo::IContactListener *listener)
{
	if (!listener)
		return;
	m_ContactListeners.emplace(listener->GetObject(), listener);
}

void G1I::PhysicsSpace::RemoveContactListener(BulletThingo::IContactListener *listener)
{
	if (!listener)
		return;

	auto range = m_ContactListeners.equal_range(listener->GetObject());
	for (auto it = range.first; it != range.second; ++it)
		if (it->second == listener)
		{
			m_ContactListeners.erase(it);
			break;
		}
}

Debug::DebugReturn G1I::PhysicsSpace::Request(Requests::Request& Action)
{
	if (Action.Name == "PausePhysics")
	{
		DWARNING("Pausing physics has been obsoleted!");
		return true;
	}
	else if (Action.Name == "ResumePhysics")
	{
		DWARNING("Pausing physics has been obsoleted!");
		return true;
	}
	else if (Action.Name == "TogglePhysics")
	{
		DWARNING("Pausing physics has been obsoleted!");
		return true;
	}
	return false;
}

void G1I::PhysicsSpace::internalphysicsevent_guy::updateAction(btCollisionWorld * world, btScalar timestep)
{
	Event::InternalPhysicsEvent e(dynamic_cast<btDynamicsWorld*>(world), timestep, *ass->UpdateID);
	ass->Event->Send(&e);
}