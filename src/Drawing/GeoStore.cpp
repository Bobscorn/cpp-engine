#include "GeoStore.h"

std::weak_ptr<GeoThing> GeoStore::GetOrCreate(std::string name, IGeoThingMaker *maker)
{
	auto it = m_Store.find(name);
	if (it != m_Store.end())
	{
		return it->second;
	}
	return m_Store.emplace(name, maker->CreateGeometry()).first->second;
}

std::weak_ptr<GeoThing> GeoStore::Query(std::string name)
{
	auto it = m_Store.find(name);
	if (it != m_Store.end())
	{
		return it->second;
	}
	return std::weak_ptr<GeoThing>();
}
