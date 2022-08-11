#pragma once

#include <memory>

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
		DrawCallv2(std::shared_ptr<Mesh> mesh, std::shared_ptr<Material> material, std::shared_ptr<Matrixy4x4> matrix, std::string debugString, bool enabled = true) : Geometry(mesh), Material(material), Matrix(matrix), DebugString(debugString), Enabled(enabled) {}
		DrawCallv2(std::string meshName, std::string materialName, std::shared_ptr<Matrixy4x4> matrix, std::string debugString, bool enabled = true) : Geometry(GeometryReference{ meshName }.GetMesh()), Material(MaterialReference{ materialName }.GetMaterial()), Matrix(matrix), DebugString(debugString), Enabled(enabled) {}
		~DrawCallv2() = default;

		DrawCallv2& operator=(DrawCallv2&& other) = default;
		DrawCallv2& operator=(const DrawCallv2& other) = default;

		std::shared_ptr<Mesh> Geometry;
		std::shared_ptr<Material> Material;
		std::string DebugString;
		std::shared_ptr<Matrixy4x4> Matrix;
		bool Enabled = true;
	};
}