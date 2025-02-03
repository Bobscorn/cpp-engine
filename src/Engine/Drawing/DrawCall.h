#pragma once

#include <memory>

#include "Math/matrix.h"

#include "GeometryReference.h"
#include "Material.h"
#include "Mesh.h"

namespace Drawing
{
	struct DrawCallv2
	{
		DrawCallv2() = default;
		DrawCallv2(DrawCallv2&& other) = default;
		DrawCallv2(const DrawCallv2& other) = default;
		DrawCallv2(std::shared_ptr<Mesh> mesh, std::shared_ptr<Material> material, std::shared_ptr<Matrixy4x4> matrix, std::string debugString, bool enabled = true) : geometry(mesh), material(material), matrix(matrix), debugString(debugString), enabled(enabled) {}
		DrawCallv2(std::string meshName, std::string materialName, std::shared_ptr<Matrixy4x4> matrix, std::string debugString, bool enabled = true) : geometry(GeometryReference{ meshName }.GetMesh()), material(MaterialReference{ materialName }.GetMaterial()), matrix(matrix), debugString(debugString), enabled(enabled) {}
		~DrawCallv2() = default;

		DrawCallv2& operator=(DrawCallv2&& other) = default;
		DrawCallv2& operator=(const DrawCallv2& other) = default;

		std::shared_ptr<Mesh> geometry;
		std::shared_ptr<Material> material;
		std::string debugString;
		std::shared_ptr<Matrixy4x4> matrix;
		bool enabled = true;
	};
}