#pragma once

#include "Helpers/VectorHelper.h"
#include "Helpers/GLHelper.h"

#include "VertexBuffer.h"

#include "Mesh.h"

#include <unordered_map>
#include <vector>
#include <string>
#include <memory>

namespace Drawing
{
	struct RawMeshData
	{
		std::string Name;
		MeshStorageType Storage;
		RawMesh Mesh;
	};

	// First -Store type class draft
	// Will use as Basic *-Store template
	class GeometryStore
	{
		// Static access
		static std::unique_ptr<GeometryStore> _instance;
	public:
		// Possibly code smell, allows going GeometryStore::Instance().GetMesh(x) or auto ref = GeometryStore::Instance;
		class Accessor
		{
		public:
			inline GeometryStore& operator()() const { return *_instance; }
			inline operator GeometryStore& () const { return *_instance; }
		} static Instance;

		// Member variables
	private:
		std::unordered_map<std::string, std::shared_ptr<Mesh>> _store;

		// Member functions

		void LoadGeometries(std::string directory);
		void LoadGeometryFromFile(std::string fileName);
	public:
		// Initializes the GeometryStore with the geometry files in geometryDirectory
		// Sets the _instance static reference;
		GeometryStore(std::string geometryDirectory, std::vector<RawMeshData> initialMeshes);

		std::shared_ptr<Mesh> GetMesh(std::string meshName);

		static void InitializeStore(std::string directory, std::vector<RawMeshData> initialMeshes);
	};
}