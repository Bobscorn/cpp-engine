#pragma once

#include <BulletCollision/CollisionDispatch/btCollisionObject.h>
#include <BulletCollision/NarrowPhaseCollision/btManifoldPoint.h>

namespace BulletThingo
{
	struct IContactListener
	{
		virtual ~IContactListener() {}

		inline btCollisionObject *GetObject() const { return m_ObjectListen; }

		virtual void OnContact(const btManifoldPoint &point) = 0;

	protected:
		btCollisionObject *m_ObjectListen = nullptr;
	};
}