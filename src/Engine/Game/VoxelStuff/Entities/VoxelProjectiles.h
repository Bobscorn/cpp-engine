#pragma once

#include "Math/floaty.h"

#include "Game/VoxelStuff/VoxelDamage.h"

#include <BulletDynamics/Dynamics/btRigidBody.h>

class btCollisionWorld;

namespace Voxel
{
	struct Entity;

	struct Projectile
	{
		Projectile(Entity *source, DamageDescription desc) : m_Lifetime(0.f), m_Source(source), m_Damage(desc), m_ShouldDie(false) { m_Damage.Type = PROJECTILE; }
		virtual ~Projectile() {}

		virtual floaty3 GetDirection() const = 0; // Not normalized
		virtual void Deflect(floaty3 new_direction, floaty3 hit_point) = 0; // Should be callable inside an Update call
		virtual void Update(btCollisionWorld *world, float delta_time) = 0;

		inline void Die() { m_ShouldDie = true; }
		inline bool ShouldDie() const { return m_ShouldDie; }

	protected:
		float m_Lifetime;
		Entity *m_Source;
		DamageDescription m_Damage;
		bool m_ShouldDie;
	};

	struct RayProjectile : Projectile
	{
		RayProjectile(floaty3 vel, floaty3 pos, float distance, Entity *source, DamageDescription desc) : Projectile(source, desc), m_Velocity(vel), m_Position(pos), m_Distance(distance), m_Gravity(true) {}
		RayProjectile(RayProjectile &&other) : RayProjectile(other.m_Velocity, other.m_Position, other.m_Distance, other.m_Source, other.m_Damage) { m_Lifetime = other.m_Lifetime; m_ShouldDie = other.m_ShouldDie; m_Gravity = other.m_Gravity; }

		floaty3 m_Velocity;
		floaty3 m_Position;
		float m_Distance;

		inline RayProjectile &operator=(RayProjectile &&other)
		{
			m_Velocity = other.m_Velocity;
			m_Position = other.m_Position;
			m_Distance = other.m_Distance;
			m_Gravity = other.m_Gravity;
			m_Lifetime = other.m_Lifetime;
			m_Source = other.m_Source;
			m_Damage = other.m_Damage;
			m_ShouldDie = other.m_ShouldDie;
			return *this;
		}

		virtual floaty3 GetDirection() const override;
		virtual void Deflect(floaty3 new_direction, floaty3 hit_point) override;
		virtual void Update(btCollisionWorld *world, float delta_time) override;

		bool m_Gravity = true;
	};

	//struct RigidBodyProjectile : Projectile
	//{
		// fk this rn
	//};

	// Projectile that travels instantly
	// Only the first update call does anything (as it travels instantly)
	// A default length of 2048m is travelled instantly, if it hits nothing (maybe something is 2049m away) it will still think it went off into space
	struct HitScanProjectile : Projectile
	{
		HitScanProjectile(floaty3 direction, floaty3 start, Entity *source, DamageDescription desc) : Projectile(source, desc), m_Direction(direction), m_Start(start), m_DiePoint(start + direction * TravelDistance) {}
		HitScanProjectile(HitScanProjectile &&other) : HitScanProjectile(other.m_Direction, other.m_Start, other.m_Source, other.m_Damage) { m_Lifetime = other.m_Lifetime; m_ShouldDie = other.m_ShouldDie; }

		inline HitScanProjectile &operator=(HitScanProjectile &&other)
		{
			m_Direction = other.m_Direction;
			m_Start = other.m_Start;
			m_DiePoint = other.m_DiePoint;
			m_Lifetime = other.m_Lifetime;
			m_Source = other.m_Source;
			m_Damage = other.m_Damage;
			m_ShouldDie = other.m_ShouldDie;
			return *this;
		}

		floaty3 m_Direction;
		floaty3 m_Start;
		floaty3 m_DiePoint;

		virtual floaty3 GetDirection() const override;
		virtual void Deflect(floaty3 new_direction, floaty3 hit_point) override;
		virtual void Update(btCollisionWorld *world, float delta_time) override;

		constexpr static float TravelDistance = 2048.f;
	};
}