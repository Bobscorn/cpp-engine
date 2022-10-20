#include "Geometry.h"

#include "Helpers/DebugPrintHelper.h"

namespace Drawing
{
	constexpr std::array<Full3DVertex, 24> CubeVertices = {
			   Full3DVertex{ floaty3(+0.500000f, +0.500000f, +0.500000f), floaty3(-0.000000f, -1.000000f, +0.000000f), floaty3(-1.000000f, +0.000000f, +0.000000f), floaty3(+0.000000f, +0.000000f, +1.000000f), floaty2(+0.000000f, +1.000000f) },
			   Full3DVertex{ floaty3(-0.500000f, -0.500000f, +0.500000f), floaty3(-0.000000f, -1.000000f, +0.000000f), floaty3(-1.000000f, +0.000000f, +0.000000f), floaty3(+0.000000f, +0.000000f, +1.000000f), floaty2(+1.000000f, +0.000000f) },
			   Full3DVertex{ floaty3(-0.500000f, +0.500000f, +0.500000f), floaty3(-0.000000f, -1.000000f, +0.000000f), floaty3(-1.000000f, +0.000000f, +0.000000f), floaty3(+0.000000f, +0.000000f, +1.000000f), floaty2(+0.000000f, +0.000000f) },
			   Full3DVertex{ floaty3(-0.500000f, +0.500000f, -0.500000f), floaty3(-1.000000f, +0.000000f, +0.000000f), floaty3(-0.000000f, -1.000000f, +0.000000f), floaty3(+0.000000f, +0.000000f, -1.000000f), floaty2(+1.000000f, +1.000000f) },
			   Full3DVertex{ floaty3(+0.500000f, -0.500001f, -0.500000f), floaty3(-1.000000f, +0.000000f, +0.000000f), floaty3(-0.000000f, -1.000000f, +0.000000f), floaty3(+0.000000f, +0.000000f, -1.000000f), floaty2(+0.000000f, +0.000000f) },
			   Full3DVertex{ floaty3(+0.500000f, +0.500000f, -0.500000f), floaty3(-1.000000f, +0.000000f, +0.000000f), floaty3(-0.000000f, -1.000000f, +0.000000f), floaty3(+0.000000f, +0.000000f, -1.000000f), floaty2(+0.000000f, +1.000000f) },
			   Full3DVertex{ floaty3(+0.500000f, +0.500000f, -0.500000f), floaty3(+0.000000f, -0.000000f, -1.000000f), floaty3(-0.000000f, -1.000000f, +0.000000f), floaty3(+1.000000f, +0.000000f, +0.000000f), floaty2(+1.000000f, +1.000000f) },
			   Full3DVertex{ floaty3(+0.500000f, -0.500000f, +0.500000f), floaty3(+0.000000f, -0.000000f, -1.000000f), floaty3(-0.000000f, -1.000000f, +0.000000f), floaty3(+1.000000f, +0.000000f, +0.000000f), floaty2(+0.000000f, +0.000000f) },
			   Full3DVertex{ floaty3(+0.500000f, +0.500000f, +0.500000f), floaty3(+0.000000f, -0.000000f, -1.000000f), floaty3(+0.000000f, -1.000000f, +0.000000f), floaty3(+1.000000f, +0.000000f, +0.000000f), floaty2(+0.000000f, +1.000000f) },
			   Full3DVertex{ floaty3(+0.500000f, -0.500001f, -0.500000f), floaty3(-0.000000f, -0.000000f, -1.000000f), floaty3(-1.000000f, +0.000000f, +0.000000f), floaty3(+0.000000f, -1.000000f, +0.000000f), floaty2(+1.000000f, +1.000000f) },
			   Full3DVertex{ floaty3(-0.500000f, -0.500000f, +0.500000f), floaty3(-0.000000f, -0.000000f, -1.000000f), floaty3(-1.000000f, +0.000000f, +0.000000f), floaty3(+0.000000f, -1.000000f, +0.000000f), floaty2(+0.000000f, +0.000000f) },
			   Full3DVertex{ floaty3(+0.500000f, -0.500000f, +0.500000f), floaty3(-0.000000f, -0.000000f, -1.000000f), floaty3(-1.000000f, +0.000000f, +0.000000f), floaty3(+0.000000f, -1.000000f, +0.000000f), floaty2(+0.000000f, +1.000000f) },
			   Full3DVertex{ floaty3(-0.500000f, -0.500000f, +0.500000f), floaty3(-0.000000f, +0.000000f, -1.000000f), floaty3(+0.000000f, +1.000000f, -0.000000f), floaty3(-1.000000f, +0.000000f, +0.000000f), floaty2(+0.000000f, +1.000000f) },
			   Full3DVertex{ floaty3(-0.500000f, +0.500000f, -0.500000f), floaty3(-0.000000f, +0.000000f, -1.000000f), floaty3(+0.000000f, +1.000000f, -0.000000f), floaty3(-1.000000f, +0.000000f, +0.000000f), floaty2(+1.000000f, +0.000000f) },
			   Full3DVertex{ floaty3(-0.500000f, +0.500000f, +0.500000f), floaty3(-0.000000f, +0.000000f, -1.000000f), floaty3(+0.000000f, +1.000000f, -0.000000f), floaty3(-1.000000f, +0.000000f, +0.000000f), floaty2(+0.000000f, +0.000000f) },
			   Full3DVertex{ floaty3(+0.500000f, +0.500000f, +0.500000f), floaty3(+0.000000f, +0.000000f, +1.000000f), floaty3(-1.000000f, +0.000000f, -0.000000f), floaty3(+0.000000f, +1.000000f, -0.000000f), floaty2(+1.000000f, +1.000000f) },
			   Full3DVertex{ floaty3(-0.500000f, +0.500000f, -0.500000f), floaty3(+0.000000f, +0.000000f, +1.000000f), floaty3(-1.000000f, +0.000000f, -0.000000f), floaty3(+0.000000f, +1.000000f, -0.000000f), floaty2(+0.000000f, +0.000000f) },
			   Full3DVertex{ floaty3(+0.500000f, +0.500000f, -0.500000f), floaty3(+0.000000f, +0.000000f, +1.000000f), floaty3(-1.000000f, +0.000000f, -0.000000f), floaty3(+0.000000f, +1.000000f, -0.000000f), floaty2(+0.000000f, +1.000000f) },
			   Full3DVertex{ floaty3(+0.500000f, -0.500000f, +0.500000f), floaty3(+0.000000f, -1.000000f, +0.000000f), floaty3(-1.000000f, +0.000000f, +0.000000f), floaty3(+0.000000f, +0.000000f, +1.000000f), floaty2(+1.000000f, +1.000000f) },
			   Full3DVertex{ floaty3(-0.500000f, -0.500000f, -0.500000f), floaty3(-1.000000f, +0.000001f, +0.000000f), floaty3(+0.000000f, -1.000000f, +0.000000f), floaty3(+0.000000f, +0.000000f, -1.000000f), floaty2(+1.000000f, +0.000000f) },
			   Full3DVertex{ floaty3(+0.500000f, -0.500001f, -0.500000f), floaty3(-0.000000f, -0.000000f, -1.000000f), floaty3(-0.000001f, -1.000000f, +0.000000f), floaty3(+1.000000f, -0.000001f, -0.000000f), floaty2(+1.000000f, +0.000000f) },
			   Full3DVertex{ floaty3(-0.500000f, -0.500000f, -0.500000f), floaty3(-0.000000f, +0.000000f, -1.000000f), floaty3(-1.000000f, +0.000000f, +0.000000f), floaty3(-0.000000f, -1.000000f, -0.000000f), floaty2(+1.000000f, +0.000000f) },
			   Full3DVertex{ floaty3(-0.500000f, -0.500000f, -0.500000f), floaty3(-0.000000f, +0.000000f, -1.000000f), floaty3(+0.000000f, +1.000000f, -0.000000f), floaty3(-1.000000f, +0.000000f, +0.000000f), floaty2(+1.000000f, +1.000000f) },
			   Full3DVertex{ floaty3(-0.500000f, +0.500000f, +0.500000f), floaty3(+0.000000f, -0.000000f, +1.000000f), floaty3(-1.000000f, +0.000000f, +0.000000f), floaty3(+0.000000f, +1.000000f, -0.000000f), floaty2(+1.000000f, +0.000000f) }
	};

	constexpr std::array<unsigned int, 36> CubeIndices = { 2, 1, 0, 5, 4, 3, 8, 7, 6, 11, 10, 9, 14, 13, 12, 17, 16, 15, 1, 18, 0, 4, 19, 3, 7, 20, 6, 10, 21, 9, 13, 22, 12, 16, 23, 15, };

	VertexData Drawing::VertexData::FromFull3DVertices(const std::vector<Full3DVertex>& vertices)
	{
		size_t numBytes = sizeof(Full3DVertex) * vertices.size();
		VertexData desc;
		desc.Description = Full3DVertexDesc;
		desc.Vertices = std::vector<char>(numBytes, (char)0);

		memcpy(desc.Vertices.data(), vertices.data(), numBytes);

		return desc;
	}

	VertexData VertexData::FromRegular2DVertices(const std::vector<Regular2DVertex>& vertices)
	{
		size_t numBytes = vertices.size() * sizeof(Regular2DVertex);
		VertexData data;
		data.Description = Regular2DVertexDesc;
		data.Vertices = std::vector<char>(numBytes, (char)0);

		memcpy(data.Vertices.data(), vertices.data(), numBytes);

		return data;
	}

	VertexData VertexData::ConvertGeometry(const VertexData& data, const GeometryDescription& to)
	{
		auto& fromDesc = data.Description;
		if (fromDesc.PositionSize != to.PositionSize ||
			fromDesc.BinormalSize != to.BinormalSize ||
			fromDesc.NormalSize != to.NormalSize ||
			fromDesc.TangentSize != to.TangentSize ||
			fromDesc.TexCoordSize != to.TexCoordSize)
		{
			DWARNING("Attempting to convert geometry from incompatible geometry types!");
			return data;
		}

		auto vertexSize = data.VertexByteSize();

		VertexData converted;
		converted.Description = to;
		converted.Vertices.resize(data.Vertices.size());

		// In Bytes
		size_t FromPosOffset = 0;
		size_t FromBinOffset = 0;
		size_t FromNorOffset = 0;
		size_t FromTanOffset = 0;
		size_t FromTexOffset = 0;

		auto fromOffsetOfLam = [&FromPosOffset, &FromBinOffset, &FromNorOffset, &FromTanOffset, &FromTexOffset](const VertexComponent& comp) -> size_t& {
			switch (comp)
			{
			default:
			case VertexComponent::POSITION: return FromPosOffset;
			case VertexComponent::TANGENT:	return FromTanOffset;
			case VertexComponent::NORMAL:	return FromNorOffset;
			case VertexComponent::BINORMAL: return FromBinOffset;
			case VertexComponent::TEXCOORD: return FromTexOffset;
			}
		};

		size_t ToPosOffset = 0;
		size_t ToBinOffset = 0;
		size_t ToNorOffset = 0;
		size_t ToTanOffset = 0;
		size_t ToTexOffset = 0;

		auto toOffsetOfLam = [&ToPosOffset, &ToBinOffset, &ToNorOffset, &ToTanOffset, &ToTexOffset](const VertexComponent& comp) -> size_t& {
			switch (comp)
			{
			default:
			case VertexComponent::POSITION: return ToPosOffset;
			case VertexComponent::TANGENT:	return ToTanOffset;
			case VertexComponent::NORMAL:	return ToNorOffset;
			case VertexComponent::BINORMAL: return ToBinOffset;
			case VertexComponent::TEXCOORD: return ToTexOffset;
			}
		};

		size_t runningOffset = 0;
		for (size_t i = 1; i < 6; ++i)
		{
			for (auto& comp : VertexComponents)
			{
				if (fromDesc.OrderOfComponent(comp) == i)
				{
					fromOffsetOfLam(comp) = runningOffset;
					runningOffset += fromDesc.SizeOfComponent(comp) * sizeof(float);
				}
			}
		}

		runningOffset = 0;
		for (size_t i = 1; i < 6; ++i)
		{
			for (auto& comp : VertexComponents)
			{
				if (to.OrderOfComponent(comp) == i)
				{
					toOffsetOfLam(comp) = runningOffset;
					runningOffset += to.SizeOfComponent(comp) * sizeof(float);
				}
			}
		}

		for (size_t i = 0; i < data.Vertices.size(); i += vertexSize)
		{
			auto* fromDat = &data.Vertices[i];
			auto* toDat = &converted.Vertices[i];
			for (auto& comp : VertexComponents)
			{
				if (fromDesc.SizeOfComponent(comp) > 0)
				{
					std::memcpy(toDat + toOffsetOfLam(comp), fromDat + fromOffsetOfLam(comp), fromDesc.SizeOfComponent(comp) * sizeof(float));
				}
			}
		}

		return converted;
	}

	size_t VertexData::NumVertices() const
	{
		size_t size = Description.GetVertexByteSize();

		if (size)
			return Vertices.size() / size;
		return 0;
	}

	size_t VertexData::VertexFloatCount() const
	{
		size_t size = 0;
		size += Description.PositionSize;
		size += Description.NormalSize;
		size += Description.BinormalSize;
		size += Description.TangentSize;
		size += Description.TexCoordSize;
		return size;
	}

	RawMesh CreateCubeMesh()
	{
		RawMesh mesh;
		mesh.VertexData = VertexData::FromFull3DVertices(CubeVertices.cbegin(), CubeVertices.cend());

		mesh.Indices = std::vector<unsigned int>(CubeIndices.cbegin(), CubeIndices.cend());
		return mesh;
	}
}

#ifdef CPP_ENGINE_TESTS

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "GeometryGeneration.h"
#include "MeshView.h"

TEST(DrawingTests, ConvertMeshTest)
{
	using namespace Drawing;
	using namespace GeoGen;
	using namespace testing;

	auto sphereMesh = GeometryGenerator::CreateSphere(0.5f, 12, 8);

	auto size = sphereMesh.Vertices.size();
	EXPECT_THAT(size, Not(0));
	EXPECT_THAT(size, Gt(10));

	auto pos1 = sphereMesh.Vertices[0].PosL;
	auto tan1 = sphereMesh.Vertices[0].TangentL;
	auto nor1 = sphereMesh.Vertices[0].NormalL;
	auto bin1 = sphereMesh.Vertices[0].BinormalL;
	auto pos2 = sphereMesh.Vertices[1].PosL;
	auto tan2 = sphereMesh.Vertices[1].TangentL;
	auto nor2 = sphereMesh.Vertices[1].NormalL;
	auto bin2 = sphereMesh.Vertices[1].BinormalL;
	auto pos10 = sphereMesh.Vertices[10].PosL;
	auto tan10 = sphereMesh.Vertices[10].TangentL;
	auto nor10 = sphereMesh.Vertices[10].NormalL;
	auto bin10 = sphereMesh.Vertices[10].BinormalL;

	auto newForm = VertexData::FromDescription(sphereMesh.Vertices, LegacyVertexDesc);

	auto meshForm = Drawing::RawMesh{ newForm, sphereMesh.Indices };
	auto meshView = MeshView<FullVertex>(meshForm);

	EXPECT_EQ(newForm.NumVertices(), size);
	EXPECT_EQ(meshView.size(), size);
	EXPECT_EQ(meshView[0].PosL, pos1);
	EXPECT_EQ(meshView[0].TangentL, tan1);
	EXPECT_EQ(meshView[0].NormalL, nor1);
	EXPECT_EQ(meshView[0].BinormalL, bin1);
	EXPECT_EQ(meshView[1].PosL, pos2);
	EXPECT_EQ(meshView[1].TangentL, tan2);
	EXPECT_EQ(meshView[1].NormalL, nor2);
	EXPECT_EQ(meshView[1].BinormalL, bin2);
	EXPECT_EQ(meshView[10].PosL, pos10);
	EXPECT_EQ(meshView[10].TangentL, tan10);
	EXPECT_EQ(meshView[10].NormalL, nor10);
	EXPECT_EQ(meshView[10].BinormalL, bin10);

	auto convertedForm = VertexData::ConvertGeometry(newForm, Full3DVertexDesc);

	auto convertedMeshForm = Drawing::RawMesh{ convertedForm, sphereMesh.Indices };
	auto convertedMeshView = MeshView<Full3DVertex>(convertedMeshForm);

	EXPECT_EQ(convertedForm.NumVertices(), size);
	EXPECT_EQ(convertedMeshView.size(), size);
	EXPECT_EQ(convertedMeshView[0].Position, pos1);
	EXPECT_EQ(convertedMeshView[0].Tangent, tan1);
	EXPECT_EQ(convertedMeshView[0].Normal, nor1);
	EXPECT_EQ(convertedMeshView[0].Binormal, bin1);
	EXPECT_EQ(convertedMeshView[1].Position, pos2);	
	EXPECT_EQ(convertedMeshView[1].Tangent, tan2);
	EXPECT_EQ(convertedMeshView[1].Normal, nor2);
	EXPECT_EQ(convertedMeshView[1].Binormal, bin2);
	EXPECT_EQ(convertedMeshView[10].Position, pos10);
	EXPECT_EQ(convertedMeshView[10].Tangent, tan10);
	EXPECT_EQ(convertedMeshView[10].Normal, nor10);
	EXPECT_EQ(convertedMeshView[10].Binormal, bin10);
}



#endif // CPP_ENGINE_TESTS
