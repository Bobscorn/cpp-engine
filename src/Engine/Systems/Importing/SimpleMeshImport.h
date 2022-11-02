#pragma once

#include "Drawing/Graphics3D.h"

#include <vector>
#include <string>


namespace Importing
{
	struct SimpleMesh
	{
		std::string MeshName;
		std::vector<FullVertex> Vertices;
		std::vector<unsigned int> Indices;
	};

	SimpleMesh ImportSimpleMesh(const std::string& fileName);

	std::vector<SimpleMesh> ImportAllSimpleMeshes(const std::string& fileName);
}
