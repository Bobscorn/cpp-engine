#pragma once

#include "BulletCollision/CollisionShapes/btBoxShape.h"

#include "Helpers/VectorHelper.h"

#include <memory>

/*
namespace Voxel
{
	struct VoxelWorld;

	struct HealthEntity
	{
		HealthEntity(VoxelWorld *world) : m_World(world) {}
		virtual ~HealthEntity() {}

		virtual void TakeDamage(float damage);
		virtual void Die();

		virtual float GetMaxHealth();
		virtual float GetMissingHealth();
		virtual float GetCurrentHealth();

		virtual floaty3 GetCentre() = 0;

		virtual void BeforeDraw() {};
		virtual void AfterDraw() {};

	protected:

		float m_Health = 1.f;
		float m_MaxHealth = 2.f;
		
		std::shared_ptr<btBoxShape> m_AABB;

		VoxelWorld *m_World = nullptr;
	};
}*/