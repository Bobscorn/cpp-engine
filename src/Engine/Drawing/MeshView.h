/** \file MeshView.h
* 
* Contains the MeshView class to make the RawMesh class more intellible and easier to modify.
* 
* To Use:
* An example function that takes a MeshView would look like this:
* - void LookAtMy3DVertices(MeshView<Full3DVertex, Full3DVertexDescription> view);
* or using the alias
* - void LookAtMy3DVertices(MeshView3D view);
* .
* To then use this view you can:
* - Produce a copy of the vertices vector but as a vector of the vertex's type
* - Index the view to get the vertex at a specific position
* - Count the number of vertices stored
* - Append a vertex to the end
* - Insert a vertex to a specified position
* - Erase a vertex from a specified position
* 
* \remarks The MeshView class stores a reference to a RawMesh, it is up to the user to ensure it does not outlive the RawMesh
* 
*/

#pragma once

#include "Mesh.h"

#include <vector>
#include <stdexcept>

namespace Drawing
{
	template<class T>
	struct MeshView
	{
		MeshView(RawMesh& mesh) : mesh(mesh) { if (sizeof(T) != mesh.vertexData.Description.GetVertexByteSize()) throw std::runtime_error("Mesh view type does not match mesh's!"); }

		RawMesh& mesh;

		std::vector<T> Copy()
		{
			size_t count = mesh.vertexData.NumVertices();
			size_t vsize = sizeof(T);
			std::vector<T> gamer{ count, T{}, std::allocator<T>() };

			std::memcpy(gamer.data(), mesh.vertexData.Vertices.data(), vsize * count);

			return gamer;
		}

		T& operator[](size_t index)
		{
			size_t vsize = sizeof(T);

			return *reinterpret_cast<T*>(&mesh.vertexData.Vertices[vsize * index]);
		}

		const T& operator[](size_t index) const
		{
			size_t vsize = sizeof(T);

			return *reinterpret_cast<const T*>(&mesh.vertexData.Vertices[vsize * index]);
		}

		size_t size() const
		{
			return mesh.vertexData.NumVertices();
		}
	};
}