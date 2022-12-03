#include "VoxelTypes.h"

#include "Helpers/MeshHelper.h"

const std::array<Voxel::BlockFace, 6> Voxel::BlockFacesArray = { Voxel::BlockFace::POS_Y, Voxel::BlockFace::NEG_Z, Voxel::BlockFace::POS_X, Voxel::BlockFace::NEG_Y, Voxel::BlockFace::POS_Z, Voxel::BlockFace::NEG_X };

const char* Voxel::GetBlockFaceName(BlockFace face)
{
	switch (face)
	{
	default: return "Unknown";
	case BlockFace::POS_Y: return "Positive Y (Up)";
	case BlockFace::NEG_Z: return "Negative Z (Forward)";
	case BlockFace::POS_X: return "Positive X (Right)";
	case BlockFace::NEG_Y: return "Negative Y (Down)";
	case BlockFace::POS_Z: return "Positive Z (Backward)";
	case BlockFace::NEG_X: return "Negative X (Left)";
	}
}

quat4 Voxel::GetWhatRotates(Voxel::BlockFace from, Voxel::BlockFace to)
{
	Vector::inty3 fromEulers;
	switch (from)
	{
	default:
	case Voxel::BlockFace::Forward: fromEulers = Vector::inty3{ 0, 0, 0 }; break;
	case Voxel::BlockFace::Back: fromEulers = Vector::inty3{ 0, 2, 0 }; break;
	case Voxel::BlockFace::Left: fromEulers = Vector::inty3{ 0, 1, 0 }; break;
	case Voxel::BlockFace::Right: fromEulers = Vector::inty3{ 0, -1, 0 }; break;
	case Voxel::BlockFace::Up: fromEulers = Vector::inty3{ 1, 0, 0 }; break;
	case Voxel::BlockFace::Down: fromEulers = Vector::inty3{ -1, 0, 0 }; break;
	}
	
	Vector::inty3 toEulers;
	switch (to)
	{
	default:
	case Voxel::BlockFace::Forward: toEulers = Vector::inty3{ 0, 0, 0 }; break;
	case Voxel::BlockFace::Back: toEulers = Vector::inty3{ 0, 2, 0 }; break;
	case Voxel::BlockFace::Left: toEulers = Vector::inty3{ 0, 1, 0 }; break;
	case Voxel::BlockFace::Right: toEulers = Vector::inty3{ 0, -1, 0 }; break;
	case Voxel::BlockFace::Up: toEulers = Vector::inty3{ 1, 0, 0 }; break;
	case Voxel::BlockFace::Down: toEulers = Vector::inty3{ -1, 0, 0 }; break;
	}
	
	quat4 inverseFrom = Voxel::GetFaceRotation(-fromEulers.x, -fromEulers.y, -fromEulers.z);
	quat4 toQuat = Voxel::GetFaceRotation(toEulers.x, toEulers.y, toEulers.z);
	
	return inverseFrom * toQuat;
}

quat4 Voxel::GetFaceRotation(int x, int y, int z)
{
	btQuaternion bQuat;
	bQuat.setEulerZYX((float)z * Math::NinetyDegreesRF, (float)y * Math::NinetyDegreesRF, (float)x * Math::NinetyDegreesRF);
	return quat4(bQuat);
}

floaty3 Voxel::BlockFaceHelper::GetDirection(BlockFace face)
{
	switch (face)
	{
	default:
	case BlockFace::Up:
		return floaty3(0, 1, 0);
	case BlockFace::Down:
		return floaty3(0, -1, 0);
	case BlockFace::Left:
		return floaty3(-1, 0, 0);
	case BlockFace::Right:
		return floaty3(1, 0, 0);
	case BlockFace::Forward:
		return floaty3(0, 0, -1);
	case BlockFace::Back:
		return floaty3(0, 0, 1);
	}
}

Vector::inty3 Voxel::BlockFaceHelper::GetDirectionI(BlockFace face)
{
	switch (face)
	{
	default:
	case BlockFace::Up:
		return Vector::inty3(0, 1, 0);
	case BlockFace::Down:
		return Vector::inty3(0, -1, 0);
	case BlockFace::Left:
		return Vector::inty3(-1, 0, 0);
	case BlockFace::Right:
		return Vector::inty3(1, 0, 0);
	case BlockFace::Forward:
		return Vector::inty3(0, 0, -1);
	case BlockFace::Back:
		return Vector::inty3(0, 0, 1);
	}
}

floaty3 Voxel::BlockFaceHelper::GetTangentDirection(BlockFace face)
{
	switch (face)
	{
	default:
	case BlockFace::Up:
		return floaty3{ +1.f, 0.f, 0.f };
	case BlockFace::Down:
		return floaty3{ -1.f, 0.f, 0.f };
	case BlockFace::Left:
		return floaty3{ 0.f, 0.f, +1.f };
	case BlockFace::Right:
		return floaty3{ 0.f, 0.f, -1.f };
	case BlockFace::Forward:
		return floaty3{ 0.f, +1.f, 0.f };
	case BlockFace::Back:
		return floaty3{ 0.f, -1.f, 0.f };
	}
}

Voxel::BlockFace Voxel::BlockFaceHelper::GetNearest(floaty3 dir)
{
	floaty3 absDir = dir.as_abs();
	if (absDir.x > absDir.y && absDir.x > absDir.z)
	{
		return dir.x > 0 ? Voxel::BlockFace::Right : Voxel::BlockFace::Left;
	}
	else if (absDir.y > absDir.z)
	{
		return dir.y > 0 ? Voxel::BlockFace::Up : Voxel::BlockFace::Down;
	}
	else
	{
		return dir.z > 0 ? Voxel::BlockFace::Back : Voxel::BlockFace::Forward;
	}
}

Voxel::BlockFace Voxel::BlockFaceHelper::GetOpposite(BlockFace face)
{
	switch (face)
	{
	default:
	case BlockFace::Right:	return BlockFace::Left;
	case BlockFace::Left:	return BlockFace::Right;
	case BlockFace::Up:		return BlockFace::Down;
	case BlockFace::Down:	return BlockFace::Up;
	case BlockFace::Forward: return BlockFace::Back;
	case BlockFace::Back:	return BlockFace::Forward;
	}
}

template<bool subtract>
Voxel::BlockCoord add_coords(const Voxel::BlockCoord& a, const Voxel::BlockCoord& b)
{
	using namespace Voxel;
	BlockCoord out;
	int64_t x_diff = (int64_t)a.Block.x + (subtract ? -(int64_t)b.Block.x : (int64_t)b.Block.x);
	int64_t y_diff = (int64_t)a.Block.y + (subtract ? -(int64_t)b.Block.y : (int64_t)b.Block.y);
	int64_t z_diff = (int64_t)a.Block.z + (subtract ? -(int64_t)b.Block.z : (int64_t)b.Block.z);
	int64_t x_adjusted_diff = x_diff < 0 ? x_diff + 1 - (int64_t)Chunk_Size : x_diff;
	int64_t y_adjusted_diff = y_diff < 0 ? y_diff + 1 - (int64_t)Chunk_Height : y_diff;
	int64_t z_adjusted_diff = z_diff < 0 ? z_diff + 1 - (int64_t)Chunk_Size : z_diff;
	auto x_chunk_dif = x_adjusted_diff / (int64_t)Chunk_Size;
	auto y_chunk_dif = y_adjusted_diff / (int64_t)Chunk_Height;
	auto z_chunk_dif = z_adjusted_diff / (int64_t)Chunk_Size;
	out.Chunk.X = a.Chunk.X + (subtract ? -b.Chunk.X : b.Chunk.X) + x_chunk_dif;
	out.Chunk.Y = a.Chunk.Y + (subtract ? -b.Chunk.Y : b.Chunk.Y) + y_chunk_dif;
	out.Chunk.Z = a.Chunk.Z + (subtract ? -b.Chunk.Z : b.Chunk.Z) + z_chunk_dif;
	out.Block.x = (uint8_t)(x_diff - x_chunk_dif * (int64_t)Chunk_Size);
	out.Block.y = (uint8_t)(y_diff - y_chunk_dif * (int64_t)Chunk_Height);
	out.Block.z = (uint8_t)(z_diff - z_chunk_dif * (int64_t)Chunk_Size);

	return out;
}

Voxel::BlockCoord Voxel::BlockCoord::operator+(const BlockCoord& other) const
{
	return add_coords<false>(*this, other);
}

Voxel::BlockCoord Voxel::BlockCoord::operator-(const BlockCoord& other) const
{
	return add_coords<true>(*this, other);
}

Voxel::BlockCoord Voxel::BlockCoord::operator-() const
{
	return BlockCoord{ ChunkCoord{ 0, 0, 0 }, 0, 0, 0 } - *this;
}

namespace MeshHelp
{
	template<>
	bool Approximately<Voxel::VoxelVertex>(const Voxel::VoxelVertex& a, const Voxel::VoxelVertex& b)
	{
		return Approximately(a.Position, b.Position)
			&& Approximately(a.Normal, b.Normal);
	}

	template bool Approximately<Voxel::VoxelVertex>(const Voxel::VoxelVertex& a, const Voxel::VoxelVertex& b);
}

#ifdef CPP_ENGINE_TESTS

#include <gtest/gtest.h>

#include "Helpers/TestHelper.h"

TEST(VoxelStuffTests, BlockCoordTests)
{
	using namespace Voxel;

	if constexpr (Chunk_Size < 8 || Chunk_Height < 8)
	{
		GTEST_SKIP() << "Chunk_Size and Chunk_Height must be at least 8 for this test to work.";
	}
	
	Voxel::BlockCoord coord{ Voxel::ChunkCoord{ 0, 0, 0 }, 1, 2, 3 };
	EXPECT_EQ(coord, (Voxel::BlockCoord{ Voxel::ChunkCoord{0, 0, 0}, 1, 2, 3 }));
	EXPECT_EQ(coord.Block.x, 1);
	EXPECT_EQ(coord.Block.y, 2);
	EXPECT_EQ(coord.Block.z, 3);

	coord = Voxel::BlockCoord{ Voxel::ChunkCoord{ 0, 0, 0 }, 5, 5, 5 };
	auto coord2 = Voxel::BlockCoord{ Voxel::ChunkCoord{ 0, 0, 0 }, 10, 10, 10 };
	EXPECT_EQ(coord, (Voxel::BlockCoord{ Voxel::ChunkCoord{0, 0, 0}, 5, 5, 5 }));
	EXPECT_EQ(coord.Block.x, 5);
	EXPECT_EQ(coord.Block.y, 5);
	EXPECT_EQ(coord.Block.z, 5);
	EXPECT_EQ(coord2, (Voxel::BlockCoord{ Voxel::ChunkCoord{0, 0, 0}, 10, 10, 10 }));
	EXPECT_EQ(coord2.Block.x, 10);
	EXPECT_EQ(coord2.Block.y, 10);
	EXPECT_EQ(coord2.Block.z, 10);
	
	EXPECT_EQ(coord - coord2, (BlockCoord{ ChunkCoord{ -1, -1, -1 }, Chunk_Size - 5, Chunk_Height - 5, Chunk_Size - 5 }));
	EXPECT_EQ(coord2 - coord, (BlockCoord{ ChunkCoord{ 0, 0, 0 }, 5, 5, 5 }));
	auto sum = (BlockCoord{ ChunkCoord{ (10 + 5) / Chunk_Size, (10 + 5) / Chunk_Height, (10 + 5) / Chunk_Size }, (10 + 5) % Chunk_Size, (10 + 5) % Chunk_Height, (10 + 5) % Chunk_Size });
	EXPECT_EQ(coord + coord2, sum);
	EXPECT_EQ(coord2 + coord, sum);

	coord = Voxel::BlockCoord{ Voxel::ChunkCoord{ 0, 0, 0 }, Chunk_Size * 2, Chunk_Height * 2, Chunk_Size * 2 };
	coord2 = Voxel::BlockCoord{ Voxel::ChunkCoord{ 0, 0, 0 }, 0, 0, 0 };
	EXPECT_EQ(coord, (Voxel::BlockCoord{ Voxel::ChunkCoord{0, 0, 0}, Chunk_Size * 2, Chunk_Height * 2, Chunk_Size * 2 }));
	EXPECT_EQ(coord.Block.x, Chunk_Size * 2);
	EXPECT_EQ(coord.Block.y, Chunk_Height * 2);
	EXPECT_EQ(coord.Block.z, Chunk_Size * 2);
	EXPECT_EQ(coord2, (Voxel::BlockCoord{ Voxel::ChunkCoord{0, 0, 0}, 0, 0, 0 }));
	
	EXPECT_EQ(coord + coord2, (BlockCoord{ ChunkCoord{ 2, 2, 2 }, 0, 0, 0 }));
	EXPECT_EQ(coord2 + coord, (BlockCoord{ ChunkCoord{ 2, 2, 2 }, 0, 0, 0 }));
	EXPECT_EQ(coord - coord2, (BlockCoord{ ChunkCoord{ 2, 2, 2 }, 0, 0, 0 }));
	auto sub = coord2 - coord;
	EXPECT_EQ(sub, (BlockCoord{ ChunkCoord{ -2, -2, -2 }, 0, 0, 0 }));

	EXPECT_EQ(-coord, (BlockCoord{ ChunkCoord{ -2, -2, -2 }, 0, 0, 0 }));

	coord = BlockCoord{ ChunkCoord{ 1, 1, 1 }, Chunk_Size * 2, Chunk_Height * 2, Chunk_Size * 2 };
	EXPECT_EQ(coord - coord2, (BlockCoord{ ChunkCoord{ 3, 3, 3 }, 0, 0, 0 }));
	EXPECT_EQ(coord2 - coord, (BlockCoord{ ChunkCoord{ -3, -3, -3 }, 0, 0, 0 }));
	EXPECT_EQ(coord + coord2, (BlockCoord{ ChunkCoord{ 3, 3, 3 }, 0, 0, 0 }));
	EXPECT_EQ(coord2 + coord, (BlockCoord{ ChunkCoord{ 3, 3, 3 }, 0, 0, 0 }));
	EXPECT_EQ(-coord, (BlockCoord{ ChunkCoord{ -3, -3, -3 }, 0, 0, 0 }));
	
	

	coord = BlockCoord{ ChunkCoord{ 5, 2, 9 }, Chunk_Size * 2 - 5, Chunk_Height * 2 + 3, Chunk_Size * 2 - 7 };
	EXPECT_EQ(coord - coord2, (BlockCoord{ ChunkCoord{ 6, 4, 10 }, Chunk_Size - 5, 3, Chunk_Size - 7 }));
	EXPECT_EQ(coord2 - coord, (BlockCoord{ ChunkCoord{ -7, -5, -11 }, 5, Chunk_Height - 3, 7 }));
}

TEST(VoxelStuffTests, GetFaceRotationTests)
{
	using namespace Voxel;
	using namespace TmpName::Testing;

	quat4 rot = GetFaceRotation(0, 1, 0);

	EXPECT_TRUE(rot.rotate(floaty3(+1.f, 0.f, 0.f)).nearly_equal(floaty3(0.f, 0.f, -1.f)));
	EXPECT_TRUE(rot.rotate(floaty3(-1.f, 0.f, 0.f)).nearly_equal(floaty3(0.f, 0.f, +1.f)));
	EXPECT_TRUE(rot.rotate(floaty3(0.f, 0.f, +1.f)).nearly_equal(floaty3(+1.f, 0.f, 0.f)));
	EXPECT_TRUE(rot.rotate(floaty3(0.f, 0.f, -1.f)).nearly_equal(floaty3(-1.f, 0.f, 0.f)));

	quat4 rot2 = GetFaceRotation(0, -1, 0 );
	
	EXPECT_TRUE(rot2.rotate(floaty3(+1.f, 0.f, 0.f)).nearly_equal(floaty3(0.f, 0.f, +1.f)));
	EXPECT_TRUE(rot2.rotate(floaty3(-1.f, 0.f, 0.f)).nearly_equal(floaty3(0.f, 0.f, -1.f)));
	EXPECT_TRUE(rot2.rotate(floaty3(0.f, 0.f, +1.f)).nearly_equal(floaty3(-1.f, 0.f, 0.f)));
	EXPECT_TRUE(rot2.rotate(floaty3(0.f, 0.f, -1.f)).nearly_equal(floaty3(+1.f, 0.f, 0.f)));
	
	quat4 rot3 = rot2 * rot;
	
	EXPECT_TRUE(ApproximatelyEquals(rot3.w, 1.f));
	EXPECT_TRUE(rot3.rotate(floaty3(1.f, 0.f, 0.f)).nearly_equal(floaty3(1.f, 0.f, 0.f)));
	EXPECT_TRUE(rot3.rotate(floaty3(0.f, 1.f, 0.f)).nearly_equal(floaty3(0.f, 1.f, 0.f)));
	EXPECT_TRUE(rot3.rotate(floaty3(0.f, 0.f, 1.f)).nearly_equal(floaty3(0.f, 0.f, 1.f)));

	rot = GetFaceRotation(1, 0, 0);
	
	EXPECT_TRUE(rot.rotate(floaty3(1.f, 0.f, 0.f)).nearly_equal(floaty3(1.f, 0.f, 0.f)));
	EXPECT_TRUE(rot.rotate(floaty3(0.f, 1.f, 0.f)).nearly_equal(floaty3(0.f, 0.f, 1.f)));

	BlockFace face = BlockFace::Forward;
	
	EXPECT_TRUE(BlockFaceHelper::GetDirection(face).nearly_equal(floaty3(0.f, 0.f, -1.f)));
	EXPECT_EQ(BlockFaceHelper::GetOpposite(face), BlockFace::Back);
	
	EXPECT_TRUE(BlockFaceHelper::GetDirection(BlockFace::Right).nearly_equal(floaty3(1.f, 0.f, 0.f)));
	EXPECT_TRUE(BlockFaceHelper::GetDirection(BlockFace::Left).nearly_equal(floaty3(-1.f, 0.f, 0.f)));
	EXPECT_TRUE(BlockFaceHelper::GetDirection(BlockFace::Up).nearly_equal(floaty3(0.f, 1.f, 0.f)));
	EXPECT_TRUE(BlockFaceHelper::GetDirection(BlockFace::Down).nearly_equal(floaty3(0.f, -1.f, 0.f)));
	EXPECT_TRUE(BlockFaceHelper::GetDirection(BlockFace::Forward).nearly_equal(floaty3(0.f, 0.f, -1.f)));
	EXPECT_TRUE(BlockFaceHelper::GetDirection(BlockFace::Back).nearly_equal(floaty3(0.f, 0.f, +1.f)));

	EXPECT_EQ(RotateFace(BlockFace::Forward,	rot), BlockFace::Up);
	EXPECT_EQ(RotateFace(BlockFace::Up,			rot), BlockFace::Back);
	EXPECT_EQ(RotateFace(BlockFace::Back,		rot), BlockFace::Down);
	EXPECT_EQ(RotateFace(BlockFace::Down,		rot), BlockFace::Forward);

	EXPECT_EQ(BlockFaceHelper::GetNearest(BlockFaceHelper::GetDirection(BlockFace::Right)), BlockFace::Right);
	EXPECT_EQ(BlockFaceHelper::GetNearest(BlockFaceHelper::GetDirection(BlockFace::Left)), BlockFace::Left);
	EXPECT_EQ(BlockFaceHelper::GetNearest(BlockFaceHelper::GetDirection(BlockFace::Up)), BlockFace::Up);
	EXPECT_EQ(BlockFaceHelper::GetNearest(BlockFaceHelper::GetDirection(BlockFace::Down)), BlockFace::Down);
	EXPECT_EQ(BlockFaceHelper::GetNearest(BlockFaceHelper::GetDirection(BlockFace::Forward)), BlockFace::Forward);
	EXPECT_EQ(BlockFaceHelper::GetNearest(BlockFaceHelper::GetDirection(BlockFace::Back)), BlockFace::Back);
}

#endif