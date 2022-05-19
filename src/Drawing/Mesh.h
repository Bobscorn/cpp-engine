#pragma once

#include "VertexBuffer.h"
#include "Geometry.h"

#include <array>
#include <vector>
#include <memory>

namespace Drawing
{
	

	struct MeshStorage
	{
		std::shared_ptr<VertexBuffer> Buffer;
		size_t ID;

		inline operator bool() const { return (bool)Buffer; }
	};

	enum class MeshStorageType
	{
		STATIC_BUFFER = 0, // Store the mesh in the universal static buffer
		DEDICATED_BUFFER = 1, // Store the mesh in its own dedicated buffer

		CUSTOM = 2, // Don't store the mesh (ie currently does nothing)
	};

	class Mesh
	{
		std::shared_ptr<RawMesh> _meshData;

		MeshStorage _storage;
	public:
		Mesh() = default;
		Mesh(const RawMesh& mesh, MeshStorageType storage);

		~Mesh() = default;

		inline operator bool() const { return _meshData && _storage.Buffer; }

		inline const MeshStorage& GetStorage() const { return _storage; }
	};
}