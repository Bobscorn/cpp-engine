#pragma once

#include <string>
#include <memory>

#include "Mesh.h"

namespace Drawing
{
	// Handles getting the actual mesh from the GeometryStore
	class GeometryReference
	{
		std::string _meshName;
		std::shared_ptr<Mesh> _mesh;

	public:
		GeometryReference() = default;
		GeometryReference(std::string meshName);
		GeometryReference(const GeometryReference& other) = default;
		GeometryReference(GeometryReference&& other);

		inline bool HasMesh() const { return _mesh.get(); }

		std::shared_ptr<Mesh> GetMesh();

		GeometryReference& operator=(const GeometryReference& other);
		GeometryReference& operator=(GeometryReference&& other);

		static std::shared_ptr<Mesh> ResolveFromName(std::string name);
	};
}