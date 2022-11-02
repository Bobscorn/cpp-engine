#pragma once

#include <array>
#include <stdexcept>
#include <cstring>
#include <vector>

#include "Math/floaty.h"
#include "GeometryDescription.h"

namespace Drawing
{
	struct Full3DVertex
	{
		floaty3 Position;
		floaty3 Normal;
		floaty3 Binormal;
		floaty3 Tangent;
		floaty2 TexCoord;

		inline bool operator==(const Full3DVertex& other) const
		{
			return Position	== other.Position
				&& Normal	== other.Normal
				&& Binormal == other.Binormal
				&& Tangent	== other.Tangent
				&& TexCoord == other.TexCoord;
		}

		inline bool operator!=(const Full3DVertex& other) const
		{
			return !(*this == other);
		}
	};

	struct Regular2DVertex
	{
		floaty2 Position;
		floaty2 TexCoord;
	};

	constexpr GeometryDescription PositionOnly3DDesc{ 3, 1, 0, 0, 0, 0, 0, 0, 0, 0 };
	constexpr GeometryDescription Full3DVertexDesc{ 3, 1, 3, 4, 3, 2, 3, 3, 2, 5 };
	constexpr GeometryDescription LegacyVertexDesc{ 3, 1, 3, 2, 3, 4, 3, 3, 2, 5 };
	constexpr GeometryDescription Regular2DVertexDesc{ 2, 1, 0, 0, 0, 0, 0, 0, 2, 2 };

	struct VertexData
	{
		std::vector<char> Vertices;

		GeometryDescription Description;

		static VertexData FromFull3DVertices(const std::vector<Full3DVertex>& vertices);
		template<typename Iter>
		static VertexData FromFull3DVertices(Iter begin, Iter end)
		{
			VertexData desc;
			desc.Description = Full3DVertexDesc;
			desc.Vertices = std::vector<char>();

			for (Iter it = begin; it != end; ++it)
			{
				desc.Vertices.insert(desc.Vertices.end(), sizeof(Full3DVertex), (char)0);
				Full3DVertex* vert = (reinterpret_cast<Full3DVertex*>(&desc.Vertices.back() + 1) - 1);
				*vert = *it;
			}

			return desc;
		}
		static VertexData FromRegular2DVertices(const std::vector<Regular2DVertex>& vertices);

		template<class T>
		static VertexData FromDescription(const std::vector<T>& vertices, GeometryDescription desc)
		{
			VertexData data;
			data.Description = desc;
			data.Vertices = std::vector<char>(vertices.size() * sizeof(T), (char)0, std::allocator<char>());

			for (int i = 0; i < vertices.size(); ++i)
			{
				std::memcpy(&data.Vertices[i * sizeof(T)], &vertices[i], sizeof(T));
			}

			return data;
		}

		template<class Iter, class Type = Iter::value_type>
		static VertexData FromGeneric(const GeometryDescription& geoDesc, Iter begin, Iter end)
		{
			if (geoDesc.GetVertexByteSize() != sizeof(Type))
				throw std::runtime_error("Exception when producing VertexData: GeometryDescription does not give the same size as the Type's size!");

			auto size = sizeof(Type);

			VertexData desc;
			desc.Description = geoDesc;
			desc.Vertices = std::vector<char>();
			desc.Vertices.reserve(std::distance(begin, end) * size);

			for (Iter it = begin; it != end; ++it)
			{
				desc.Vertices.insert(desc.Vertices.end(), size, (char)0);
				Type* vert = (reinterpret_cast<Type*>(&desc.Vertices.back() + 1) - 1);
				*vert = *it;
			}

			return desc;
		}

		static VertexData ConvertGeometry(const VertexData& data, const GeometryDescription& to);

		size_t NumVertices() const;
		size_t VertexFloatCount() const;
		inline constexpr size_t VertexByteSize() const { return Description.GetVertexByteSize(); }
	};

	struct RawMesh
	{
		VertexData VertexData;

		std::vector<unsigned int> Indices;
	};

	RawMesh CreateCubeMesh();
}