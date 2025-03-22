#pragma once

#include <unordered_map>
#include <optional>

#include "Drawing/Graphics1.h"
#include "VoxelTypes.h"
#include "VoxelChunk.h"

namespace Voxel
{
	/*
	 * Defines a class to store a VoxelWorld's serialized chunk data in memory, rather than on disk.
	 * 
	 */
	class VoxelMemoryLevel : public IChunkMemory
	{
		std::unordered_map<ChunkCoord, std::unique_ptr<ChunkData>> m_StoredChunks;

	public:
		VoxelMemoryLevel();

		void SetChunkData(ChunkCoord coord, std::unique_ptr<ChunkData> data) override;
		std::unique_ptr<ChunkData> GetChunkData(ChunkCoord coord) const override;

		void Reset() override;
	};
}
