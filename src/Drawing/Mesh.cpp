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
}