#include "GeometryStore.h"

#include <cstring>

namespace Drawing
{
	std::unique_ptr<GeometryStore> GeometryStore::_instance = nullptr;
	GeometryStore::Accessor GeometryStore::Instance{};

	void eat_ass()
	{
		Drawing::GeometryStore::Instance();
	}

	void GeometryStore::LoadGeometries(std::string directory)
	{
		// TODO: load geometries and crap
	}

	void GeometryStore::LoadGeometryFromFile(std::string fileName)
	{
		// TODO: load geometries from file
		// e.g.
		// Assimp::importer uwu(fileName);
		// auto storage = MeshStorageType::Dynamic;
		// if (fileName.contains("static"))
		//		storage = MeshStorageType::Static;
		// 
		// LoadMesh(uwu->mesh, storage);
		//
	}

	GeometryStore::GeometryStore(std::string geometryDirectory, std::vector<RawMeshData> initialMeshes)
	{
		for (auto& mesh : initialMeshes)
			_store[mesh.Name] = std::make_shared<Mesh>();
		LoadGeometries(geometryDirectory);
	}

	std::shared_ptr<Mesh> GeometryStore::GetMesh(std::string meshName)
	{
		auto it = _store.find(meshName);
		if (it != _store.end())
			return it->second;
		return nullptr;
	}

	void GeometryStore::InitializeStore(std::string directory, std::vector<RawMeshData> initialMeshes)
	{
		_instance = std::make_unique<GeometryStore>(directory, initialMeshes);
	}
}
