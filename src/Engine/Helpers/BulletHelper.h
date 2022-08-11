#pragma once

#include <BulletCollision/CollisionShapes/btSphereShape.h>
#include <BulletCollision/CollisionShapes/btBoxShape.h>

#include <ratio>
#include <memory>

namespace BulletHelp
{
	struct INothingInterface;

	struct NothingHolder
	{
		INothingInterface *Pointy;
	};

	struct INothingInterface
	{
		INothingInterface() : m_Holder{ this } {}
		virtual ~INothingInterface() {}

		inline virtual void DoNothing() {}

	protected:
		NothingHolder m_Holder;
	};
}

template<intmax_t num, intmax_t den>
struct SphereCreator
{
	static std::shared_ptr<btSphereShape> GetShape()
	{
		auto ptr = m_Shape.lock();
		if (ptr)
			return ptr;

		ptr = std::make_shared<btSphereShape>(static_cast<float>(std::ratio<num, den>::num) / static_cast<float>(std::ratio<num, den>::den));
		m_Shape = ptr;
		return ptr;
	}

protected:
	static std::weak_ptr<btSphereShape> m_Shape;
};

template<intmax_t num, intmax_t den>
std::weak_ptr<btSphereShape> SphereCreator<num, den>::m_Shape = std::weak_ptr<btSphereShape>();