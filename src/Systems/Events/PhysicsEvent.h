#include "EventsBase.h"
#include "Converter.h"

#pragma warning(push, 0)
#include <BulletDynamics/Dynamics/btDynamicsWorld.h>
#pragma warning(pop)

namespace Event
{
	struct BeforePhysicsEvent : public Events::IEvent
	{
		constexpr static Events::Event MyType = Events::Event::BeforePhysicsEvent;
		BeforePhysicsEvent(btDynamicsWorld *world, unsigned int frameid = 0u) : IEvent(MyType, frameid), World(world) {}
		btDynamicsWorld *World;
		inline bool operator==(Events::IEvent *other) override { return (Events::ConvertEvent<BeforePhysicsEvent>(other)->World == this->World); }
	};

	struct InternalPhysicsEvent : Events::IEvent
	{
		constexpr static Events::Event MyType = Events::Event::InternalPhysicsEvent;
		InternalPhysicsEvent(btDynamicsWorld *world, btScalar timestep, unsigned int frameid = 0) : Events::IEvent(MyType, frameid), world(world), timestep(timestep) {};
		btDynamicsWorld *world;
		btScalar timestep;
		inline bool operator==(Events::IEvent * other) { auto *e = Events::ConvertEvent<InternalPhysicsEvent>(other); if (!e) return false; return (e->world == this->world && timestep == e->timestep); }
	};

	struct AfterPhysicsEvent : public Events::IEvent
	{
		constexpr static Events::Event MyType = Events::Event::AfterPhysicsEvent;
		AfterPhysicsEvent(btDynamicsWorld *world, unsigned int frameid = 0) : IEvent(MyType, frameid), World(world) {}
		btDynamicsWorld *World;
		inline bool operator==(Events::IEvent *other) override { auto *e = Events::ConvertEvent<AfterPhysicsEvent>(other); if (!e) return false; return (e->World == this->World); }
	};
}