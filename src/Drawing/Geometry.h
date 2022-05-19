#pragma once

#include <array>

#include "Helpers/VectorHelper.h"
#include "Helpers/GLHelper.h"

namespace Drawing
{
	enum class VertexComponent
	{
		POSITION = 0,
		TANGENT,
		NORMAL,
		BINORMAL,
		TEXCOORD,
	};

	constexpr std::array<VertexComponent, 5> VertexComponents{ VertexComponent::POSITION, VertexComponent::TANGENT, VertexComponent::NORMAL, VertexComponent::BINORMAL, VertexComponent::TEXCOORD };

	inline constexpr bool IsVectorVertexComponent(VertexComponent comp)
	{
		return comp == VertexComponent::TANGENT
			|| comp == VertexComponent::NORMAL
			|| comp == VertexComponent::BINORMAL;
	}

	inline constexpr const char* VertexComponentName(VertexComponent comp)
	{
		switch (comp)
		{
		default:
		case VertexComponent::POSITION:
			return "Position";
		case VertexComponent::TANGENT:
			return "Tangent";
		case VertexComponent::NORMAL:
			return "Normal";
		case VertexComponent::BINORMAL:
			return "Binormal";
		case VertexComponent::TEXCOORD:
			return "Texture Coords";
		}
	}

	/// <summary>
	/// <para>
	/// Describes how to interpret bytes as vertices.
	/// </para>
	/// <para>
	/// Assumes the bytes are laid out vertex by vertex (for eg. [position][normal][tex][position][normal][tex]...)
	/// </para>
	/// <para>
	/// Sizes are in number of floats (floating point data is assumed), an Order of 0 is the first element, 1 is the 2nd, etc...
	/// </para>
	/// </summary>
	struct GeometryDescription
	{
		constexpr GeometryDescription() : PositionSize(0), PositionOrder(0), TangentSize(0), TangentOrder(0), NormalSize(0), NormalOrder(0), BinormalSize(0), BinormalOrder(0), TexCoordSize(0), TexCoordOrder(0) {}
		constexpr GeometryDescription(size_t posS, size_t posO, size_t tanS, size_t tanO, size_t normS, size_t normO, size_t binormS, size_t binormO, size_t texS, size_t texO)
			: PositionSize(posS), PositionOrder(posO), TangentSize(tanS), TangentOrder(tanO), NormalSize(normS), NormalOrder(normO), BinormalSize(binormS), BinormalOrder(binormO), TexCoordSize(texS), TexCoordOrder(texO) {}

		constexpr size_t GetVertexSize() const { return PositionSize + TangentSize + NormalSize + BinormalSize + TexCoordSize; };
		constexpr size_t GetVertexByteSize() const { return GetVertexSize() * sizeof(float); }

		size_t PositionSize;
		size_t PositionOrder; // The order of the Position vertex (0 is first, 1 is 2nd, etc.) identical to GLSL's layout(location = x)
		size_t TangentSize; // Set to zero to ignore Tangents
		size_t TangentOrder;
		size_t NormalSize; // Set to zero to ignore normals, 3 for 3D normals, 2 for 2D normals
		size_t NormalOrder;
		size_t BinormalSize; // Set to zero to ignore Binormals, 3 for 3D Binormals, 2 for 2D normals
		size_t BinormalOrder;
		size_t TexCoordSize; // Set to zero to ignore Texture Coordinates
		size_t TexCoordOrder;

		inline bool operator==(const GeometryDescription& other) const
		{
			return PositionSize == other.PositionSize
				&& PositionOrder == other.PositionOrder
				&& TangentSize == other.TangentSize
				&& TangentOrder == other.TangentOrder
				&& NormalSize == other.NormalSize
				&& NormalOrder == other.NormalOrder
				&& BinormalSize == other.BinormalSize
				&& BinormalOrder == other.BinormalOrder
				&& TexCoordSize == other.TexCoordSize
				&& TexCoordOrder == other.TexCoordOrder;
		}

		inline bool operator!=(const GeometryDescription& other) const { return !(*this == other); }

		inline constexpr size_t SizeOfComponent(VertexComponent comp) const
		{
			switch (comp)
			{
			default:
			case VertexComponent::POSITION:
				return PositionSize;
			case VertexComponent::NORMAL:
				return NormalSize;
			case VertexComponent::TANGENT:
				return TangentSize;
			case VertexComponent::BINORMAL:
				return BinormalSize;
			case VertexComponent::TEXCOORD:
				return TexCoordSize;
			}
		}
		inline constexpr size_t OrderOfComponent(VertexComponent comp) const
		{
			switch (comp)
			{
			default:
			case VertexComponent::POSITION:
				return PositionOrder;
			case VertexComponent::NORMAL:
				return NormalOrder;
			case VertexComponent::TANGENT:
				return TangentOrder;
			case VertexComponent::BINORMAL:
				return BinormalOrder;
			case VertexComponent::TEXCOORD:
				return TexCoordOrder;
			}
		}
	};

	struct Full3DVertex
	{
		floaty3 Position;
		floaty3 Normal;
		floaty3 Binormal;
		floaty3 Tangent;
		floaty2 TexCoord;
	};

	struct Regular2DVertex
	{
		floaty2 Position;
		floaty2 TexCoord;
	};

	constexpr GeometryDescription Full3DVertexDesc{ 3, 1, 3, 2, 3, 4, 3, 3, 2, 5 };
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

		size_t NumVertices() const;
		size_t VertexSize() const;
	};

	struct RawMesh
	{
		VertexData VertexData;

		std::vector<GLuint> Indices;
	};

	RawMesh CreateCubeMesh();
}