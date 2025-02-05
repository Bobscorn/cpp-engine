#pragma once

#include "Drawing/Graphics3D.h"
#include "Drawing/IRen3D.h"
#include "Drawing/GeometryGeneration.h"

#include "Drawing/FreeMaterials.h"

#include <memory>

template<intmax_t num, intmax_t den, uint32_t sliceCount = 3, uint32_t stackCount = 3>
struct GeoMan
{
	static std::shared_ptr<GeoThing> Get(IRen3D *ren)
	{
		auto ptr = m_Geo.lock();
		if (ptr)
			return ptr;

		GeoGen::MeshData dat = GeoGen::GeometryGenerator::CreateSphere((static_cast<float>(std::ratio<num,den>::num) / std::ratio<num, den>::den), sliceCount, stackCount);
		ptr = ren->AddGeometry(dat.Vertices, dat.Indices);
		m_Geo = ptr;
		return ptr;
	}

protected:
	static std::weak_ptr<GeoThing> m_Geo;
};

template<intmax_t num, intmax_t den, uint32_t sliceCount, uint32_t stackCount>
std::weak_ptr<GeoThing> GeoMan<num, den, sliceCount, stackCount>::m_Geo = std::weak_ptr<GeoThing>();

template<size_t index>
struct MatMan
{
	static std::shared_ptr<Material> Get()
	{
		auto ptr = m_Mat.lock();
		if (ptr)
			return ptr;

		ptr = std::make_shared<Material>(FreeMats[index]);
		m_Mat = ptr;
		return ptr;
	}

protected:
	static std::weak_ptr<Material> m_Mat;
};

template<size_t index>
std::weak_ptr<Material> MatMan<index>::m_Mat = std::weak_ptr<Material>();