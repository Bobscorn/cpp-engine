#include "VoxelMemoryLevel.h"

namespace Voxel
{
	VoxelMemoryLevel::VoxelMemoryLevel()
	{
	}

	void VoxelMemoryLevel::SetChunkData(ChunkCoord coord, std::unique_ptr<ChunkData> data)
	{
		m_StoredChunks[coord] = std::move(data);
	}

	std::unique_ptr<ChunkData> VoxelMemoryLevel::GetChunkData(ChunkCoord coord) const
	{
		auto it = m_StoredChunks.find(coord);
		if (it == m_StoredChunks.end())
			return nullptr;
		return std::make_unique<ChunkData>(*it->second);
	}

	void VoxelMemoryLevel::Reset()
	{
		m_StoredChunks.clear();
	}
}
