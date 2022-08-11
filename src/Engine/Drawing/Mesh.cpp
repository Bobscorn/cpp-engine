#include "Mesh.h"

namespace Drawing
{
	Mesh::Mesh(const RawMesh& mesh, MeshStorageType storage)
		: _meshData(std::make_shared<RawMesh>(mesh))
	{
		switch (storage)
		{
		case MeshStorageType::STATIC_BUFFER:
			_storage.Buffer = VertexBuffer::GetStaticBuffer();
			_storage.ID = _storage.Buffer->AddMesh(mesh);
			break;
		case MeshStorageType::DEDICATED_BUFFER:
			_storage.Buffer = std::make_shared<VertexBuffer>();
			_storage.ID = _storage.Buffer->AddMesh(mesh);
			break;

		default:
		case MeshStorageType::CUSTOM:
			break;
		}
	}

	Mesh::Mesh(Mesh&& other)
		: _storage(std::move(other._storage))
		, _meshData(std::move(other._meshData))
	{
		other._storage.Buffer = nullptr;
		other._storage.ID = 0;

		other._meshData = nullptr;
	}

	void Mesh::Reset()
	{
		_storage.Buffer = nullptr;
		_storage.ID = 0;
		_meshData = nullptr;
	}

	Mesh& Mesh::operator=(Mesh&& other)
	{
		Reset();

		_storage = std::move(other._storage);
		_meshData = std::move(other._meshData);

		other._storage.Buffer = nullptr;
		other._storage.ID = 0;
		other._meshData = nullptr;

		return *this;
	}
}