#include "ShapeStore.h"

#include "BulletCollision/CollisionShapes/btBoxShape.h"

/*std::weak_ptr<btCollisionShape> ShapeStore::GetOrCreate(std::string name, IShapeCreator *maker)
{
	auto it = m_Store.find(name);
	if (it != m_Store.end())
	{
		return it->second;
	}
	return m_Store.emplace(name, maker->CreateShape()).first->second;
}

std::weak_ptr<btCollisionShape> ShapeStore::Query(std::string name)
{
	auto it = m_Store.find(name);
	if (it != m_Store.end())
	{
		return it->second;
	}
	return std::weak_ptr<btCollisionShape>();
}
*/