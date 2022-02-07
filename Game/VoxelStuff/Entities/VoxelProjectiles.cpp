#include "VoxelProjectiles.h"

#include "Structure/BulletBitmasks.h"

#include "Helpers/BulletHelper.h"

#include "Game/VoxelStuff/VoxelAbility.h"

#include <BulletCollision/CollisionDispatch/btCollisionWorld.h>


floaty3 Voxel::HitScanProjectile::GetDirection() const
{
	return m_Direction;
}

void Voxel::HitScanProjectile::Deflect(floaty3 new_direction, floaty3 hit_point)
{
	m_ShouldDie = false;

	m_Start = hit_point;
	m_Direction = new_direction;
}

void Voxel::HitScanProjectile::Update(btCollisionWorld *world, float delta_time)
{
	m_Lifetime += delta_time;

	auto &far = m_DiePoint = m_Start + m_Direction * TravelDistance;

	btCollisionWorld::ClosestRayResultCallback callback(m_Start, far);

	callback.m_collisionFilterGroup = PROJECTILE_GENERAL;
	callback.m_collisionFilterMask = ENTITY_GENERAL | ENVIRONMENT | PLAYER;

	world->rayTest(m_Start, far, callback);

	if (callback.hasHit())
	{
		m_DiePoint = floaty3(callback.m_hitPointWorld);
		auto hit = callback.m_collisionObject;
		if (hit->getUserPointer())
		{
			auto holder = reinterpret_cast<BulletHelp::NothingHolder*>(hit->getUserPointer());
			auto entitties = dynamic_cast<Voxel::Entity *>(holder->Pointy);
			if (entitties)
			{
				Die(); // entitties->HitByProjectile may call this->Deflect, which will reset m_ShouldDie to false to keep it alive,  calling Die() before means ensures that this reset is not ignored
				if (entitties->HitByProjectile(this, floaty3(callback.m_hitPointWorld)))
				{
					if (m_Source)
						m_Source->AbilityAttack(entitties, m_Damage);
					else
						entitties->TakeDamage(m_Damage, nullptr);
				}
			}
			else
			{
				Die();
			}
		}
		else
		{
			Die();
		}
	}
	else
	{
		Die();
	}
}

floaty3 Voxel::RayProjectile::GetDirection() const
{
	return m_Velocity;
}

void Voxel::RayProjectile::Deflect(floaty3 new_direction, floaty3 hit_point)
{
	m_Position = hit_point;
	float mag = m_Velocity.magnitude();
	m_Velocity = new_direction * mag;
}

void Voxel::RayProjectile::Update(btCollisionWorld *world, float delta_time)
{
	m_Lifetime += delta_time;
	// Gravity
	if (m_Gravity)
		m_Velocity += floaty3{ 0.f, -9.8f, 0.f } *delta_time;

	floaty3 next_position = m_Position + m_Velocity * delta_time;
	floaty3 end = next_position + floaty3::SafelyNormalized(m_Velocity) * m_Distance;

	btCollisionWorld::ClosestRayResultCallback callback{m_Position, end};
	callback.m_collisionFilterGroup = PROJECTILE_GENERAL;
	callback.m_collisionFilterMask = ENTITY_GENERAL | ENVIRONMENT | PLAYER;

	world->rayTest(m_Position, end, callback);

	if (callback.hasHit())
	{
		if (callback.m_collisionObject->getUserPointer())
		{
			auto holder = reinterpret_cast<BulletHelp::NothingHolder*>(callback.m_collisionObject->getUserPointer());
			auto entity = dynamic_cast<Entity *>(holder->Pointy);
			if (entity)
			{
				if (entity->HitByProjectile(this, floaty3(callback.m_hitPointWorld)))
				{
					if (m_Source)
						m_Source->AbilityAttack(entity, m_Damage);
					else
						entity->TakeDamage(m_Damage, nullptr);
				}
			}
		}
	}
	else
	{
		m_Position = next_position;
	}
}
