#include "VoxelChunk.h"

#include "Structure/BulletBitmasks.h"

constexpr floaty3 OriginFromCoord(Voxel::ChunkCoord coord)
{
	return { ((float)Voxel::Chunk_Size * -0.5f) - 0.5f + (float)((int64_t)Voxel::Chunk_Size * coord.X), ((float)Voxel::Chunk_Size * -0.5f) - 0.5f + (float)((int64_t)Voxel::Chunk_Size * coord.Y), ((float)Voxel::Chunk_Size * -0.5f) - 0.5f + (float)((int64_t)Voxel::Chunk_Size * coord.Z) };
}

constexpr floaty3 PositionFromCoord(floaty3 origin, size_t x, size_t y, size_t z)
{
	return { origin.x + ((float)x + 0.5f) * Voxel::BlockSize, origin.y + ((float)y + 0.5f) * Voxel::BlockSize, origin.z + ((float)z + 0.5f) * Voxel::BlockSize };
}

Voxel::ChunkyBoi::ChunkyBoi(G1::IGSpace *container, CommonResources *resources, VoxelWorld *world, floaty3 origin) : G1::IShape(container), FullResourceHolder(resources), m_World(world), m_Culler(CreateCuller(m_Origin)), m_Origin(origin)
{
}

Voxel::ChunkyBoi::ChunkyBoi(G1::IGSpace *container, CommonResources *resources, VoxelWorld *world, floaty3 origin, RawChunkData initial_dat) : G1::IShape(container), FullResourceHolder(resources), m_World(world), m_Culler(CreateCuller(m_Origin)), m_Origin(origin), m_Data()
{
	for (size_t x = 0; x < Chunk_Size; ++x)
	{
		for (size_t y = 0; y < Chunk_Height; ++y)
		{
			for (size_t z = 0; z < Chunk_Size; ++z)
			{
				if (initial_dat[x][y][z].ID)
					create(x, y, z);
			}
		}
	}
}

Voxel::ChunkyBoi::ChunkyBoi(G1::IGSpace *container, CommonResources *resources, VoxelWorld *world, floaty3 origin, RawChunkDataMap initial_dat) : G1::IShape(container), FullResourceHolder(resources), m_World(world), m_Culler(CreateCuller(m_Origin)), m_Origin(origin), m_Data()
{
	for (auto &pair : initial_dat)
	{
		if (pair.second.ID)
		{
			create(pair.first.X, pair.first.Y, pair.first.Z);
		}
	}
}

Voxel::ChunkyBoi::~ChunkyBoi()
{
}

void Voxel::ChunkyBoi::BeforeDraw()
{
	if (m_Culler)
		m_Culler->Flush();

	for (auto &plane : m_Data)
	{
		for (auto &line : plane)
		{
			for (auto &block : line)
			{
				if (block)
					block->BeforeDraw();
			}
		}
	}
}

void Voxel::ChunkyBoi::AfterDraw()
{


	for (auto &plane : m_Data)
	{
		for (auto &line : plane)
		{
			for (auto &block : line)
			{
				if (block)
					block->AfterDraw();
			}
		}
	}
}

void Voxel::ChunkyBoi::UpdateOrigin(floaty3 origin)
{
	m_Origin = origin;
	m_Culler = CreateCuller(m_Origin);
	for (size_t x = 0; x < Chunk_Size; ++x)
	{
		for (size_t y = 0; y < Chunk_Height; ++y)
		{
			for (size_t z = 0; z < Chunk_Size; ++z)
			{
				auto &block = m_Data[x][y][z];
				if (block)
					block->UpdatePosition(PositionFromCoord(m_Origin, x, y, z), x, y, z, m_Culler.get());
			}
		}
	}
}

Voxel::ChunkyFrustumCuller *Voxel::ChunkyBoi::GetCuller() const
{
	return m_Culler.get();
}

void Voxel::ChunkyBoi::set(size_t x, size_t y, size_t z, std::unique_ptr<Voxel::ICube> val)
{
	m_Data[x][y][z] = std::move(val);
}

void Voxel::ChunkyBoi::create(size_t x, size_t y, size_t z)
{
	m_Data[x][y][z] = std::make_unique<Voxel::VoxelCube>(this->Container, this->mResources, this->m_World, PositionFromCoord(m_Origin, x, y, z), x, y, z, m_Culler.get());
}

void Voxel::ChunkyBoi::SetTo(RawChunkData data)
{
	std::unordered_map<std::string, std::shared_ptr<GeoThing>> geos;
	std::unordered_map<std::string, std::shared_ptr<Material>> mats;
	std::unordered_map<std::string, std::shared_ptr<btCollisionShape>> shapes;
}

std::unique_ptr<Voxel::ChunkyFrustumCuller> Voxel::ChunkyBoi::CreateCuller(floaty3 origin)
{
	return std::make_unique<Voxel::ChunkyFrustumCuller>(origin, floaty3{ Voxel::Chunk_Size, Voxel::Chunk_Height, Voxel::Chunk_Size });
}
