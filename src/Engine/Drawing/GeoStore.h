#pragma once

#include "Graphics3D.h"

#include <unordered_map>
#include <string>

struct IGeoThingMaker
{
	virtual ~IGeoThingMaker() {}

	virtual const std::shared_ptr<GeoThing> &CreateGeometry() = 0;
};

struct GeoStore
{

	std::weak_ptr<GeoThing> GetOrCreate(std::string name, IGeoThingMaker *maker);
	std::weak_ptr<GeoThing> Query(std::string name);

protected:

	std::unordered_map<std::string, std::weak_ptr<GeoThing>> m_Store;
};