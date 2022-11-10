#include "VoxelCube.h"

#include "Helpers/ProfileHelper.h"

#include "Drawing/IRen3D.h"

#include "Structure/BulletBitmasks.h"

#include "Drawing/VoxelStore.h"
#include "Game/VoxelStuff/VoxelChunk.h"

//std::weak_ptr<btBoxShape> Voxel::VoxelCube::s_Shape;

constexpr std::array<FullVertex, 24> CubeVertices = {
			FullVertex{ floaty3(+0.500000f, +0.500000f, +0.500000f), floaty3(-0.000000f, -1.000000f, +0.000000f), floaty3(-1.000000f, +0.000000f, +0.000000f), floaty3(+0.000000f, +0.000000f, +1.000000f), floaty2(+0.000000f, +1.000000f) },
			FullVertex{ floaty3(-0.500000f, -0.500000f, +0.500000f), floaty3(-0.000000f, -1.000000f, +0.000000f), floaty3(-1.000000f, +0.000000f, +0.000000f), floaty3(+0.000000f, +0.000000f, +1.000000f), floaty2(+1.000000f, +0.000000f) },
			FullVertex{ floaty3(-0.500000f, +0.500000f, +0.500000f), floaty3(-0.000000f, -1.000000f, +0.000000f), floaty3(-1.000000f, +0.000000f, +0.000000f), floaty3(+0.000000f, +0.000000f, +1.000000f), floaty2(+0.000000f, +0.000000f) },
			FullVertex{ floaty3(-0.500000f, +0.500000f, -0.500000f), floaty3(-1.000000f, +0.000000f, +0.000000f), floaty3(-0.000000f, -1.000000f, +0.000000f), floaty3(+0.000000f, +0.000000f, -1.000000f), floaty2(+1.000000f, +1.000000f) },
			FullVertex{ floaty3(+0.500000f, -0.500001f, -0.500000f), floaty3(-1.000000f, +0.000000f, +0.000000f), floaty3(-0.000000f, -1.000000f, +0.000000f), floaty3(+0.000000f, +0.000000f, -1.000000f), floaty2(+0.000000f, +0.000000f) },
			FullVertex{ floaty3(+0.500000f, +0.500000f, -0.500000f), floaty3(-1.000000f, +0.000000f, +0.000000f), floaty3(-0.000000f, -1.000000f, +0.000000f), floaty3(+0.000000f, +0.000000f, -1.000000f), floaty2(+0.000000f, +1.000000f) },
			FullVertex{ floaty3(+0.500000f, +0.500000f, -0.500000f), floaty3(+0.000000f, -0.000000f, -1.000000f), floaty3(-0.000000f, -1.000000f, +0.000000f), floaty3(+1.000000f, +0.000000f, +0.000000f), floaty2(+1.000000f, +1.000000f) },
			FullVertex{ floaty3(+0.500000f, -0.500000f, +0.500000f), floaty3(+0.000000f, -0.000000f, -1.000000f), floaty3(-0.000000f, -1.000000f, +0.000000f), floaty3(+1.000000f, +0.000000f, +0.000000f), floaty2(+0.000000f, +0.000000f) },
			FullVertex{ floaty3(+0.500000f, +0.500000f, +0.500000f), floaty3(+0.000000f, -0.000000f, -1.000000f), floaty3(+0.000000f, -1.000000f, +0.000000f), floaty3(+1.000000f, +0.000000f, +0.000000f), floaty2(+0.000000f, +1.000000f) },
			FullVertex{ floaty3(+0.500000f, -0.500001f, -0.500000f), floaty3(-0.000000f, -0.000000f, -1.000000f), floaty3(-1.000000f, +0.000000f, +0.000000f), floaty3(+0.000000f, -1.000000f, +0.000000f), floaty2(+1.000000f, +1.000000f) },
			FullVertex{ floaty3(-0.500000f, -0.500000f, +0.500000f), floaty3(-0.000000f, -0.000000f, -1.000000f), floaty3(-1.000000f, +0.000000f, +0.000000f), floaty3(+0.000000f, -1.000000f, +0.000000f), floaty2(+0.000000f, +0.000000f) },
			FullVertex{ floaty3(+0.500000f, -0.500000f, +0.500000f), floaty3(-0.000000f, -0.000000f, -1.000000f), floaty3(-1.000000f, +0.000000f, +0.000000f), floaty3(+0.000000f, -1.000000f, +0.000000f), floaty2(+0.000000f, +1.000000f) },
			FullVertex{ floaty3(-0.500000f, -0.500000f, +0.500000f), floaty3(-0.000000f, +0.000000f, -1.000000f), floaty3(+0.000000f, +1.000000f, -0.000000f), floaty3(-1.000000f, +0.000000f, +0.000000f), floaty2(+0.000000f, +1.000000f) },
			FullVertex{ floaty3(-0.500000f, +0.500000f, -0.500000f), floaty3(-0.000000f, +0.000000f, -1.000000f), floaty3(+0.000000f, +1.000000f, -0.000000f), floaty3(-1.000000f, +0.000000f, +0.000000f), floaty2(+1.000000f, +0.000000f) },
			FullVertex{ floaty3(-0.500000f, +0.500000f, +0.500000f), floaty3(-0.000000f, +0.000000f, -1.000000f), floaty3(+0.000000f, +1.000000f, -0.000000f), floaty3(-1.000000f, +0.000000f, +0.000000f), floaty2(+0.000000f, +0.000000f) },
			FullVertex{ floaty3(+0.500000f, +0.500000f, +0.500000f), floaty3(+0.000000f, +0.000000f, +1.000000f), floaty3(-1.000000f, +0.000000f, -0.000000f), floaty3(+0.000000f, +1.000000f, -0.000000f), floaty2(+1.000000f, +1.000000f) },
			FullVertex{ floaty3(-0.500000f, +0.500000f, -0.500000f), floaty3(+0.000000f, +0.000000f, +1.000000f), floaty3(-1.000000f, +0.000000f, -0.000000f), floaty3(+0.000000f, +1.000000f, -0.000000f), floaty2(+0.000000f, +0.000000f) },
			FullVertex{ floaty3(+0.500000f, +0.500000f, -0.500000f), floaty3(+0.000000f, +0.000000f, +1.000000f), floaty3(-1.000000f, +0.000000f, -0.000000f), floaty3(+0.000000f, +1.000000f, -0.000000f), floaty2(+0.000000f, +1.000000f) },
			FullVertex{ floaty3(+0.500000f, -0.500000f, +0.500000f), floaty3(+0.000000f, -1.000000f, +0.000000f), floaty3(-1.000000f, +0.000000f, +0.000000f), floaty3(+0.000000f, +0.000000f, +1.000000f), floaty2(+1.000000f, +1.000000f) },
			FullVertex{ floaty3(-0.500000f, -0.500000f, -0.500000f), floaty3(-1.000000f, +0.000001f, +0.000000f), floaty3(+0.000000f, -1.000000f, +0.000000f), floaty3(+0.000000f, +0.000000f, -1.000000f), floaty2(+1.000000f, +0.000000f) },
			FullVertex{ floaty3(+0.500000f, -0.500001f, -0.500000f), floaty3(-0.000000f, -0.000000f, -1.000000f), floaty3(-0.000001f, -1.000000f, +0.000000f), floaty3(+1.000000f, -0.000001f, -0.000000f), floaty2(+1.000000f, +0.000000f) },
			FullVertex{ floaty3(-0.500000f, -0.500000f, -0.500000f), floaty3(-0.000000f, +0.000000f, -1.000000f), floaty3(-1.000000f, +0.000000f, +0.000000f), floaty3(-0.000000f, -1.000000f, -0.000000f), floaty2(+1.000000f, +0.000000f) },
			FullVertex{ floaty3(-0.500000f, -0.500000f, -0.500000f), floaty3(-0.000000f, +0.000000f, -1.000000f), floaty3(+0.000000f, +1.000000f, -0.000000f), floaty3(-1.000000f, +0.000000f, +0.000000f), floaty2(+1.000000f, +1.000000f) },
			FullVertex{ floaty3(-0.500000f, +0.500000f, +0.500000f), floaty3(+0.000000f, -0.000000f, +1.000000f), floaty3(-1.000000f, +0.000000f, +0.000000f), floaty3(+0.000000f, +1.000000f, -0.000000f), floaty2(+1.000000f, +0.000000f) }
};
constexpr std::array<unsigned int, 36> CubeIndices = { 2, 1, 0, 5, 4, 3, 8, 7, 6, 11, 10, 9, 14, 13, 12, 17, 16, 15, 1, 18, 0, 4, 19, 3, 7, 20, 6, 10, 21, 9, 13, 22, 12, 16, 23, 15, };

Voxel::VoxelCube::VoxelCube(G1::IGSpace *container, CommonResources *resources, VoxelWorld *world, VoxelChunk* chunk, ChunkBlockCoord coord) 
	: IShape(container, "Cube")
	, ICube(world, chunk, coord)
	, FullResourceHolder(resources)
{
	PROFILE_PUSH("Construct Body");
	//btTransform tmp;
	//tmp.setFromOpenGLMatrix(m_Trans.ma);
	//m_Body->setWorldTransform(tmp);

	PROFILE_PUSH("Request Physics");
	//Container->RequestPhysicsCall(m_Body, ENVIRONMENT, PLAYER | ENTITY_GENERAL);
	PROFILE_POP();
	PROFILE_POP();
}

Voxel::VoxelCube::~VoxelCube()
{
}

void Voxel::VoxelCube::BeforeDraw()
{
}

void Voxel::VoxelCube::AfterDraw()
{
}

std::unique_ptr<Voxel::ICube> Voxel::VoxelCube::Clone(VoxelWorld* world, VoxelChunk* chunk, ChunkBlockCoord pos) const
{
	return std::make_unique<VoxelCube>(GetContainer(), mResources, world, chunk, pos);
}

void Voxel::VoxelCube::ResetCuller()
{
}

Voxel::CubeID Voxel::ICube::GetBlockID() const
{
	if (!m_Chunk)
		return 0;
	return m_Chunk->get_data(m_Pos).ID;
}

Voxel::BlockCoord Voxel::ICube::GetWorldPos() const
{
	return (m_Chunk ? BlockCoord{ m_Chunk->GetCoord(), m_Pos } : BlockCoord{});
}

const std::string& Voxel::ICube::GetBlockName() const
{
	return VoxelStore::Instance().GetNameOf(GetBlockID());
}

const Voxel::SerialBlock& Voxel::ICube::GetBlockData() const
{
	static SerialBlock EmptyData{};

	if (!m_Chunk)
	{
		if (m_Data)
			return *m_Data;
		return EmptyData;
	}
	return m_Chunk->get_data(m_Pos);
}

Voxel::SerialBlock Voxel::ICube::TakeBlockData()
{
	static SerialBlock EmptyData{};

	if (m_Data)
	{
		SerialBlock tmp = std::move(*m_Data);
		m_Data = nullptr;
		return tmp;
	}
	return EmptyData;
}
