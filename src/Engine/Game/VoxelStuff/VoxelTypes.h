#pragma once

#include <cstdint>

namespace Voxel
{
	struct ChunkCoord
	{
		int64_t X;
		int64_t Y;
		int64_t Z;

		inline bool operator==(const ChunkCoord &other) const
		{
			return X == other.X && Y == other.Y && Z == other.Z;
		}
	};

	struct BlockCoord
	{
		ChunkCoord Chunk;
		size_t X, Y, Z;
	};
}