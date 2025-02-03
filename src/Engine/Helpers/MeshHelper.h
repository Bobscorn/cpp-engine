#pragma once

#include "Drawing/Mesh.h"
#include "Drawing/MeshView.h"

#include <unordered_map>
#include <vector>
#include <algorithm>

namespace MeshHelp
{
	// Explicitly instantiated in cpp file
	template<class T>
	bool Approximately(const T& a, const T& b);

	template<class T>
	Drawing::RawMesh DeDuplicateVertices(Drawing::MeshView<T> mesh)
	{
		std::vector<T> uniqueVertices{};
		std::vector<std::pair<T, GLuint>> vertexLookup{};
		uniqueVertices.reserve(mesh.size());
		vertexLookup.reserve(mesh.size());

		GLuint next = 0;

		// Collect all unique vertices into a vector, and store their *new* index in the vertexLookup map
		for (int i = 0; i < mesh.size(); ++i)
		{
			auto& vert = mesh[i];

			bool nonUnique = false;
			for (int j = 0; j < uniqueVertices.size(); ++j)
			{
				nonUnique |= Approximately(vert, uniqueVertices[j]);
			}

			if (!nonUnique && std::find_if(vertexLookup.begin(), vertexLookup.end(), [&vert](const std::pair<T, GLuint>& p) { return p.first == vert; }) == vertexLookup.end())
			{
				uniqueVertices.push_back(vert);
				vertexLookup.push_back(std::make_pair(vert, next));
				++next;
			}
		}

		// Convert old indices to new indices
		std::vector<GLuint> newIndices{ mesh.mesh.Indices.size(), 0, std::allocator<GLuint>() };
		for (int i = 0; i < newIndices.size(); ++i)
		{
			auto oldIndex = mesh.mesh.Indices[i];
			auto vert = mesh[oldIndex];
			auto it = std::find_if(vertexLookup.begin(), vertexLookup.end(), [&vert](const std::pair<T, GLuint>& p) { return Approximately(p.first, vert); });
			if (it == vertexLookup.end())
				DERROR("Failed to find vertex in lookup");
			newIndices[i] = it->second;
		}

		return Drawing::RawMesh{ Drawing::VertexData::FromDescription(uniqueVertices, mesh.mesh.vertexData.Description), newIndices };
	}
}