#include "GeometryReference.h"

#include "GeometryStore.h"

namespace Drawing
{
	GeometryReference::GeometryReference(std::string meshName)
		: _meshName(std::move(meshName))
	{
	}

	GeometryReference::GeometryReference(GeometryReference&& other)
		: _meshName(std::move(other._meshName))
		, _mesh(std::move(other._mesh))
	{
		other._meshName = "";
		other._mesh = nullptr;
	}

	std::shared_ptr<Mesh> GeometryReference::GetMesh()
	{
		if (!_mesh && _meshName.size())
			_mesh = GeometryStore::Instance().GetMesh(_meshName);
		return _mesh;
	}

	GeometryReference& GeometryReference::operator=(const GeometryReference& other)
	{
		if (_meshName != other._meshName)
			_mesh = nullptr;

		_meshName = other._meshName;

		return *this;
	}

	GeometryReference& GeometryReference::operator=(GeometryReference&& other)
	{
		_meshName = std::move(other._meshName);
		_mesh = std::move(other._mesh);

		other._meshName.clear();
		other._mesh = nullptr;

		return *this;
	}
}