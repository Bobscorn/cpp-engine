#pragma once

#include "Helpers/MathHelper.h"
#include "Math/quat4.h"

#include "VoxelValues.h"

#include <cstdint>
#include <cmath>
#include <ostream>
#include <string>

namespace Voxel
{
	enum class BlockFace
	{
		UP = 0,
		Up = 0,
		POS_Y = 0,
		FORWARD = 1,
		Forward = 1,
		NEG_Z = 1,
		RIGHT = 2,
		Right = 2,
		POS_X = 2,
		DOWN = 3,
		Down = 3,
		NEG_Y = 3,
		BACK = 4,
		Back = 4,
		POS_Z = 4,
		LEFT = 5,
		Left = 5,
		NEG_X = 5,
	};
	
	/// <summary>
	/// Calculates a quaternion that will rotate a face facing one direction to another.
	/// </summary>
	/// <param name="from">The initial face</param>
	/// <param name="to">The target face to rotate to</param>
	/// <returns>A quat4 that will rotate a BlockFace identical to 'from' to a BlockFace identical to 'to'</returns>
	quat4 GetWhatRotates(BlockFace from, BlockFace to);
	
	/// <summary>
	/// Calculates a quaternion that will apply 90 degree rotations around X, Y and Z axes. Z rotation is applied first, then Y, then X.
	/// </summary>
	/// <param name="x">Number of times to rotate 90 degrees around the X axis</param>
	/// <param name="y">Number of times to rotate 90 degrees around the Y axis</param>
	/// <param name="z">Number of times to rotate 90 degrees around the Z axis</param>
	/// <returns>A quat4 representing the 3 rotations</returns>
	quat4 GetFaceRotation(int x, int y, int z);

	class BlockFaceHelper
	{
	public:
		static floaty3 GetDirection(BlockFace face);
		static Vector::inty3 GetDirectionI(BlockFace face);
		static BlockFace GetNearest(floaty3 dir);
		static BlockFace GetOpposite(BlockFace face);
	};
	
	// Will rotate a face by a quaternion, and take the nearest resulting face (possibly resulting in same face)
	// Intended to used with GetFaceRotation()
	inline BlockFace RotateFace(BlockFace face, quat4 rot)
	{
		floaty3 dir = BlockFaceHelper::GetDirection(face);
		floaty3 rotDir = rot.rotate(dir);
		return BlockFaceHelper::GetNearest(rotDir);
	}

	struct ChunkCoord
	{
		int64_t X;
		int64_t Y;
		int64_t Z;

		inline bool operator==(const ChunkCoord &other) const
		{
			return X == other.X && Y == other.Y && Z == other.Z;
		}

		inline ChunkCoord operator+(const ChunkCoord& other) const
		{
			return ChunkCoord{ X + other.X, Y + other.Y, Z + other.Z };
		}
	};

	struct ChunkBlockCoord
	{
		uint32_t x, y, z;
	};

	inline bool operator==(const ChunkBlockCoord& a, const ChunkBlockCoord& b)
	{
		return a.x == b.x
			&& a.y == b.y
			&& a.z == b.z;
	}

	inline bool operator!=(const ChunkBlockCoord& a, const ChunkBlockCoord& b)
	{
		return !(a == b);
	}

	struct BlockCoord
	{
		ChunkCoord Chunk;
		ChunkBlockCoord Block;

		inline bool operator==(const BlockCoord &other) const
		{
			return Chunk == other.Chunk && Block.x == other.Block.x && Block.y == other.Block.y && Block.z == other.Block.z;
		}

		inline bool operator!=(const BlockCoord &other) const
		{
			return !(*this == other);
		}

		BlockCoord operator+(const BlockCoord& other) const;
		BlockCoord operator-(const BlockCoord& other) const;
		BlockCoord operator-() const;

		static constexpr BlockCoord Origin() { return BlockCoord{ ChunkCoord{ 0, 0, 0 }, 0, 0, 0 }; }

		friend std::ostream& operator<<(std::ostream& os, const BlockCoord& bc)
		{
			os << "BlockCoord(C(" << bc.Chunk.X << ", " << bc.Chunk.Y << ", " << bc.Chunk.Z << "), " << bc.Block.x << ", " << bc.Block.y << ", " << bc.Block.z << ")";
			return os;
		}
	};


	struct CubeID
	{
		CubeID() {}
		CubeID(std::string id) : ID(std::move(id)) {}

		std::string ID;
	};

	struct CubeData
	{
		quat4 Rotation;

	};

	struct SerialBlock
	{
		CubeID ID;
		CubeData Data;
	};
}