#pragma once

#include <BulletCollision/CollisionShapes/btCollisionShape.h>

#include <string>
#include <unordered_map>
#include <memory>

/*struct IShapeCreator
{
	virtual ~IShapeCreator() {}
	// Expected to create a shared_ptr somewhere else and return copy of it
	virtual std::shared_ptr<btCollisionShape> CreateShape() = 0;
};

template<class ShapeType, class ... Args>
struct DefaultShapeCreator : IShapeCreator
{
	DefaultShapeCreator(std::shared_ptr<ShapeType> &outside_shape, Args ...args) : m_Ref(outside_shape), m_Args(args) {}

	virtual std::shared_ptr<btCollisionShape> CreateShape() override
	{
		m_Ref = std::make_shared<ShapeType>(args);

		return std::static_pointer_cast<btCollisionShape>(m_Ref);
	}

	std::shared_ptr<ShapeType> &m_Ref;
	Args ...m_Args;
};

struct ShapeStore
{

	std::weak_ptr<btCollisionShape> GetOrCreate(std::string name, IShapeCreator *maker);
	std::weak_ptr<btCollisionShape> Query(std::string name);

protected:

	std::unordered_map<std::string, std::weak_ptr<btCollisionShape>> m_Store;
};*/