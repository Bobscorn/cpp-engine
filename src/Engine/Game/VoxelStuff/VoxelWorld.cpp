#include "VoxelWorld.h"
#include "VoxelWorld.h"

#include "Helpers/MathHelper.h"
#include "Helpers/ProfileHelper.h"

#include "Systems/Events/PhysicsEvent.h"
#include "Systems/Time/Time.h"

#include "VoxelAbility.h"
#include "Drawing/VoxelStore.h"

#include <vector>
#include <algorithm>
#include <cstdlib>
#include <execution>

Voxel::SerialBlock GetBlockIdInWorld(const Voxel::VoxelWorld* world, Voxel::BlockCoord coord)
{
	return world->GetCubeDataAt(coord);
}

Voxel::VoxelWorld::VoxelWorld(G1::IShapeThings things, WorldStuff stuff)
	: IShape(things)
	, FullResourceHolder(things.Resources)
	, m_Stuff(stuff)
	, m_LoadingStuff(std::make_shared<LoadingStuff>())
	, m_LoadingThread()
	, m_ChunkLoadingOffsets(CalculateOffsets(stuff.HalfBonusWidth, stuff.HalfBonusHeight, stuff.HalfBonusDepth))
{
	m_Stuff.ChunkLeniance = Math::min<size_t>()(m_Stuff.ChunkLeniance, m_Stuff.HalfBonusWidth);
	m_Stuff.ChunkLeniance = Math::min<size_t>()(m_Stuff.ChunkLeniance, m_Stuff.HalfBonusHeight);
	m_Stuff.ChunkLeniance = Math::min<size_t>()(m_Stuff.ChunkLeniance, m_Stuff.HalfBonusDepth);

	if (m_Stuff.ChunkLeniance < 1)
		m_Stuff.ChunkLeniance = 1;

	if (m_Stuff.m_ChunkUnloader == nullptr)
		m_Stuff.m_ChunkUnloader = this;

	LoadingOtherStuff funcs;
	funcs.GetBlockIdFunc = [this](BlockCoord coord) { return GetBlockIdInWorld(this, coord); };
	funcs.GetChunkDataFunc = [gen = m_Stuff.m_ChunkLoader](ChunkCoord coord) -> std::unique_ptr<Voxel::ChunkData> { if (!gen) return {}; return ConvertMapToData(gen->LoadChunk(coord.X, coord.Y, coord.Z)); };
	m_LoadingThread = std::thread(DoChunkLoading, m_LoadingStuff, std::move(funcs));
}

Voxel::VoxelWorld::~VoxelWorld()
{
	m_LoadingStuff->QuitVal.store(true);
	m_LoadingThread.join();
}

std::vector<Voxel::ChunkCoord> Voxel::CalculateOffsets(int64_t xHalfBound, int64_t yHalfBound, int64_t zHalfBound)
{
	auto numPoints = (xHalfBound * 2 + 1) * (yHalfBound * 2 + 1) * (zHalfBound * 2 + 1);
	std::vector<Voxel::ChunkCoord> coords{};
	coords.reserve((size_t)numPoints);

	for (int64_t x = -xHalfBound; x <= xHalfBound; ++x)
	{
		for (int64_t y = -yHalfBound; y <= yHalfBound; ++y)
		{
			for (int64_t z = -zHalfBound; z <= zHalfBound; ++z)
			{
				coords.emplace_back(ChunkCoord{ x, y, z });
			}
		}
	}

	auto lessThan = [](Voxel::ChunkCoord a, Voxel::ChunkCoord b) { return (std::abs(a.X) + std::abs(a.Y) + std::abs(a.Z)) < (std::abs(b.X) + std::abs(b.Y) + std::abs(b.Z)); };

	std::sort(std::execution::par_unseq, coords.begin(), coords.end(), lessThan);

	return coords;
}

floaty3 Voxel::VoxelWorld::Update(floaty3 New_Centre)
{
	//PROFILE_PUSH("VoxelWorld Update");
	//// What to do:
	//// check if player if ChunkLeniance chunks away from the centre

	//// Convert New_Centre to World space
	//DOUBLE3 centre = (DOUBLE3)New_Centre + m_PhysicsDisplacement;

	//// Convert World space to chunk space
	//centre = { centre.x / Chunk_Width_Double, centre.y / Chunk_Tallness_Double, centre.z / Chunk_Width_Double };

	//// centre is now in world chunk space
	//ChunkCoord chunk_centre = GetChunkCoordFromPhys(New_Centre);


	//// Now create the desired cube of chunks around the centre

	//int64_t lower_x = (chunk_centre.X - (int64_t)m_Stuff.HalfBonusWidth), upper_x = (chunk_centre.X + (int64_t)m_Stuff.HalfBonusWidth);
	//int64_t lower_y = (chunk_centre.Y - (int64_t)m_Stuff.HalfBonusHeight), upper_y = (chunk_centre.Y + (int64_t)m_Stuff.HalfBonusHeight);
	//int64_t lower_z = (chunk_centre.Z - (int64_t)m_Stuff.HalfBonusDepth), upper_z = (chunk_centre.Z + (int64_t)m_Stuff.HalfBonusDepth);

	//// Unload Chunks
	//PROFILE_PUSH("Unloading Chunks");
	//std::vector<ChunkCoord> to_unload;

	//for (auto &chunk : m_Chunks)
	//{
	//	if (chunk.first.X < lower_x || chunk.first.X > upper_x ||
	//		chunk.first.Y < lower_y || chunk.first.Y > upper_y ||
	//		chunk.first.Z < lower_z || chunk.first.Z > upper_z)
	//	{
	//		to_unload.emplace_back(chunk.first);
	//	}
	//}

	//for (auto &chunkIndex : to_unload)
	//{
	//	std::unique_ptr<VoxelChunk> ptr = std::move(m_Chunks[chunkIndex]);
	//	m_Stuff.m_ChunkUnloader->UnloadChunk(std::move(ptr));
	//	m_Chunks.erase(chunkIndex);
	//}
	//PROFILE_POP();

	//// Load new chunks
	//m_Chunks.reserve(m_Stuff.HalfBonusWidth * 2 * m_Stuff.HalfBonusHeight * 2 * m_Stuff.HalfBonusHeight * 2);

	//// Convert to centre + half extents
	//const int64_t &centre_x = chunk_centre.X;
	//const int64_t &centre_y = chunk_centre.Y;
	//const int64_t &centre_z = chunk_centre.Z;

	///*for (int64_t i = 0; i <= (int64_t)m_Stuff.HalfBonusWidth; ++i)
	//{
	//	for (int64_t j = 0; j <= (int64_t)m_Stuff.HalfBonusHeight; ++j)
	//	{
	//		for (int64_t n = 0; n <= (int64_t)m_Stuff.HalfBonusDepth; ++n)
	//		{
	//			ChunkCoord point1 = { centre_x + i, centre_y + j, centre_z + n };
	//			ChunkCoord point2 = { centre_x - i, centre_y - j, centre_z - n };
	//			Load(point1);
	//			Load(point2);
	//		}
	//	}
	//}*/

	//int64_t x = 0, y = 0, z = 0;

	//bool go = true, do_x = false, do_y = false, do_z = false;

	//PROFILE_PUSH("Loading Chunks");
	//Load({ centre_x, centre_y, centre_z });

	//while (true)
	//{
	//	go = false;
	//	if (x < static_cast<int64_t>(m_Stuff.HalfBonusWidth))
	//	{
	//		++x;
	//		do_x = go = true;
	//	}
	//	if (y < static_cast<int64_t>(m_Stuff.HalfBonusHeight))
	//	{
	//		++y;
	//		do_y = go = true;
	//	}
	//	if (z < static_cast<int64_t>(m_Stuff.HalfBonusDepth))
	//	{
	//		++z;
	//		do_z = go = true;
	//	}
	//	if (!go)
	//		break;
	//	

	//	// X ---------------------------------------------------------------------
	//	if (do_x)
	//	{
	//		// Load positive and negative versions at the same time
	//		int x_y = 0, x_z = 0;
	//		int y_extent = (int)(x < static_cast<int64_t>(m_Stuff.HalfBonusHeight) ? x : m_Stuff.HalfBonusHeight);
	//		int z_extent = (int)(x < static_cast<int64_t>(m_Stuff.HalfBonusDepth) ? x : m_Stuff.HalfBonusDepth);
	//		bool x_do_y = false, x_do_z = false, x_go = false;

	//		Load({ centre_x + x, centre_y, centre_z });
	//		Load({ centre_x - x, centre_y, centre_z });

	//		while (true)
	//		{
	//			x_go = false;
	//			if (x_y < y_extent)
	//			{
	//				++x_y;
	//				x_do_y = true;
	//				x_go = true;
	//			}
	//			if (x_z < z_extent)
	//			{
	//				++x_z;
	//				x_do_z = true;
	//				x_go = true;
	//			}

	//			if (!x_go)
	//				break;

	//			// Now we have a x, and a z value to work with,
	//			// Now 'draw' the vertical lines
	//			// Z first just because chunks are taller than wide
	//			if (x_do_z)
	//			{
	//				for (int i = 0; i <= x_y; ++i)
	//				{
	//					Load({ centre_x + x, centre_y + i, centre_z + x_z });
	//					Load({ centre_x + x, centre_y + i, centre_z - x_z });
	//					Load({ centre_x + x, centre_y - i, centre_z + x_z });
	//					Load({ centre_x + x, centre_y - i, centre_z - x_z });
	//					Load({ centre_x - x, centre_y + i, centre_z + x_z });
	//					Load({ centre_x - x, centre_y + i, centre_z - x_z });
	//					Load({ centre_x - x, centre_y - i, centre_z + x_z });
	//					Load({ centre_x - x, centre_y - i, centre_z - x_z });
	//				}
	//			}

	//			// Now with the x and y values 'draw' the horizontal lines
	//			if (x_do_y)
	//			{
	//				for (int i = 0; i <= x_z; ++i)
	//				{
	//					Load({ centre_x + x, centre_y + x_y, centre_z + i });
	//					Load({ centre_x + x, centre_y + x_y, centre_z - i });
	//					Load({ centre_x + x, centre_y - x_y, centre_z + i });
	//					Load({ centre_x + x, centre_y - x_y, centre_z - i });
	//					Load({ centre_x - x, centre_y + x_y, centre_z + i });
	//					Load({ centre_x - x, centre_y + x_y, centre_z - i });
	//					Load({ centre_x - x, centre_y - x_y, centre_z + i });
	//					Load({ centre_x - x, centre_y - x_y, centre_z - i });
	//				}
	//			}
	//		}

	//		

	//	}

	//	// Y----------------------------------------------------------------------
	//	if (do_y)
	//	{
	//		// Load positive and negative versions at the same time
	//		int y_x = 0, y_z = 0;
	//		int x_extent = (int)(y < (int64_t)m_Stuff.HalfBonusWidth ? y : m_Stuff.HalfBonusWidth);
	//		int z_extent = (int)(y < (int64_t)m_Stuff.HalfBonusDepth ? y : m_Stuff.HalfBonusDepth);
	//		bool y_do_x = false, y_do_z = false, y_go = false;

	//		Load({ centre_x , centre_y + y, centre_z });
	//		Load({ centre_x , centre_y - y, centre_z });

	//		while (true)
	//		{
	//			y_go = false;
	//			if (y_x < x_extent)
	//			{
	//				++y_x;
	//				y_do_x = true;
	//				y_go = true;
	//			}
	//			if (y_z < z_extent)
	//			{
	//				++y_z;
	//				y_do_z = true;
	//				y_go = true;
	//			}

	//			if (!y_go)
	//				break;

	//			// 'Drawing' the lines with the established 2 variables
	//			if (y_do_z)
	//			{
	//				for (int i = 0; i <= y_z; ++i)
	//				{
	//					Load({ centre_x + y_x, centre_y + y, centre_z + i });
	//					Load({ centre_x + y_x, centre_y + y, centre_z - i });
	//					Load({ centre_x + y_x, centre_y - y, centre_z + i });
	//					Load({ centre_x + y_x, centre_y - y, centre_z - i });
	//					Load({ centre_x - y_x, centre_y + y, centre_z + i });
	//					Load({ centre_x - y_x, centre_y + y, centre_z - i });
	//					Load({ centre_x - y_x, centre_y - y, centre_z + i });
	//					Load({ centre_x - y_x, centre_y - y, centre_z - i });
	//				}
	//			}

	//			if (y_do_z)
	//			{
	//				for (int i = 0; i <= y_x; ++i)
	//				{
	//					Load({ centre_x + i, centre_y + y, centre_z + y_z });
	//					Load({ centre_x + i, centre_y + y, centre_z - y_z });
	//					Load({ centre_x + i, centre_y - y, centre_z + y_z });
	//					Load({ centre_x + i, centre_y - y, centre_z - y_z });
	//					Load({ centre_x - i, centre_y + y, centre_z + y_z });
	//					Load({ centre_x - i, centre_y + y, centre_z - y_z });
	//					Load({ centre_x - i, centre_y - y, centre_z + y_z });
	//					Load({ centre_x - i, centre_y - y, centre_z - y_z });
	//				}
	//			}
	//		}
	//	}

	//	// Z--------------------------------------------------------------
	//	if (do_z)
	//	{
	//		// Load positive and negative versions at the same time
	//		int z_x = 0, z_y = 0;
	//		int x_extent = (int)(z < m_Stuff.HalfBonusWidth ? z : m_Stuff.HalfBonusWidth);
	//		int y_extent = (int)(z < m_Stuff.HalfBonusHeight ? z : m_Stuff.HalfBonusHeight);
	//		bool z_do_x = false, z_do_y = false, z_go = false;

	//		Load({ centre_x , centre_y, centre_z + z });
	//		Load({ centre_x , centre_y, centre_z - z });

	//		while (true)
	//		{
	//			z_go = false;
	//			if (z_x < x_extent)
	//			{
	//				++z_x;
	//				z_do_x = true;
	//				z_go = true;
	//			}
	//			if (z_y < y_extent)
	//			{
	//				++z_y;
	//				z_do_y = true;
	//				z_go = true;
	//			}

	//			if (!z_go)
	//				break;

	//			// 'Draw' the lines
	//			if (z_do_x)
	//			{
	//				for (int i = 0; i <= z_y; ++i)
	//				{
	//					Load({ centre_x + z_x, centre_y + i, centre_z + z });
	//					Load({ centre_x + z_x, centre_y + i, centre_z - z });
	//					Load({ centre_x + z_x, centre_y - i, centre_z + z });
	//					Load({ centre_x + z_x, centre_y - i, centre_z - z });
	//					Load({ centre_x - z_x, centre_y + i, centre_z + z });
	//					Load({ centre_x - z_x, centre_y + i, centre_z - z });
	//					Load({ centre_x - z_x, centre_y - i, centre_z + z });
	//					Load({ centre_x - z_x, centre_y - i, centre_z - z });
	//				}
	//			}

	//			if (z_do_y)
	//			{
	//				for (int i = 0; i <= z_x; ++i)
	//				{
	//					Load({ centre_x + i, centre_y + z_y, centre_z + z });
	//					Load({ centre_x + i, centre_y + z_y, centre_z - z });
	//					Load({ centre_x + i, centre_y - z_y, centre_z + z });
	//					Load({ centre_x + i, centre_y - z_y, centre_z - z });
	//					Load({ centre_x - i, centre_y + z_y, centre_z + z });
	//					Load({ centre_x - i, centre_y + z_y, centre_z - z });
	//					Load({ centre_x - i, centre_y - z_y, centre_z + z });
	//					Load({ centre_x - i, centre_y - z_y, centre_z - z });
	//				}
	//			}
	//		}
	//	}
	//}

	//PROFILE_POP();

	//PROFILE_POP();

	PROFILE_PUSH("New Chunk Loading");

	auto centre = GetChunkCoordFromPhys(New_Centre);

	int64_t centre_x = centre.X;
	int64_t centre_y = centre.Y;
	int64_t centre_z = centre.Z;

	Load(centre);
	for (auto& offset : m_ChunkLoadingOffsets)
	{
		Load({ centre_x + offset.X, centre_y + offset.Y, centre_z + offset.Z });
	}

	PROFILE_POP();
	PROFILE_PUSH("Chunk Unloading");
	std::vector<ChunkCoord> to_unload;
	
	for (auto& chunkPair : m_Chunks)
	{
		auto& chunkCoord = chunkPair.first;
		
		if (chunkCoord.X > (centre_x + (int64_t)m_Stuff.HalfBonusWidth ) || chunkCoord.X < (centre_x - (int64_t)m_Stuff.HalfBonusWidth) ||
			chunkCoord.Y > (centre_y + (int64_t)m_Stuff.HalfBonusHeight) || chunkCoord.Y < (centre_y - (int64_t)m_Stuff.HalfBonusHeight) ||
			chunkCoord.Z > (centre_z + (int64_t)m_Stuff.HalfBonusDepth ) || chunkCoord.Z < (centre_z - (int64_t)m_Stuff.HalfBonusDepth))
		{
			to_unload.push_back(chunkCoord);
		}
	}
	
	for (auto& chunkCoord : to_unload)
		Unload(chunkCoord);
	
	PROFILE_POP();

	return { 0.f, 0.f, 0.f };
}

void Voxel::VoxelWorld::BeforeDraw()
{
	PROFILE_PUSH("VoxelWorld BeforeDraw");
	PROFILE_PUSH("Chunks");
	for (auto &chunk : m_Chunks)
		if (chunk.second)
			chunk.second->BeforeDraw();
	PROFILE_POP();
	PROFILE_PUSH("Entities");
	for (auto &entity : m_DynamicEntities)
		entity.second->BeforeDraw();
	PROFILE_POP();
	PROFILE_POP();
}

void Voxel::VoxelWorld::Draw()
{
	constexpr static floaty3 Base = { 1.f, 0.f, 0.f };
	constexpr static floaty3 Color = { 0.2f, 0.2f, 0.7f };
	// Draw Projectiles
	std::vector<ProjInstanceData> instance_dat;
	instance_dat.reserve(m_RayProjectiles.size() + m_HitscanProjectiles.size());
	for (auto &proj : m_RayProjectiles)
	{
		floaty3 pos = proj.m_Position;
		floaty3 dir = proj.GetDirection();
		floaty3 rot_axis = floaty3::SafelyNormalized(Base.cross(dir));
		if (rot_axis.mag2() < 1.f)
			rot_axis = { 0.f, 1.f, 0.f };
		float dot = Base.dot(dir);
		float det = Base.x * dir.y * rot_axis.z + dir.x * rot_axis.y * Base.z + rot_axis.x * Base.y * dir.z - Base.z * dir.y * rot_axis.x - dir.z * rot_axis.y * Base.x - rot_axis.z * Base.y * dir.x;
		float angle = atan2f(det, dot);;
		Matrixy3x3 rot = Matrixy3x3::RotationAxisNormR(rot_axis, angle);
		instance_dat.emplace_back(ProjInstanceData{ pos, Color, rot });
	}
}

void Voxel::VoxelWorld::AfterDraw()
{
	PROFILE_PUSH("VoxelWorld AfterDraw");
	for (auto &chunk : m_Chunks)
		if (chunk.second)
		{
			PROFILE_PUSH_AGG("Chunk");
			chunk.second->AfterDraw();
			PROFILE_POP();
		}
	PROFILE_PUSH("Entities");
	for (auto &entity : m_DynamicEntities)
		entity.second->AfterDraw();
	PROFILE_POP();

	DoRemoveEntities();
	PROFILE_PUSH("Check Loading Thread");
	CheckLoadingThread();
	//ApplyChunkChanges();
	PROFILE_POP();
	PROFILE_POP();
}

bool Voxel::VoxelWorld::Receive(Events::IEvent *e)
{
	(void)e;
	return true;
}

bool Voxel::VoxelWorld::Receive(Event::AfterPhysicsEvent *event)
{
	Time::TimeType delta_time = mResources->Time->GetDeltaTime();

	for (auto &proj : m_RayProjectiles)
		proj.Update(event->World, delta_time);

	for (auto &proj : m_HitscanProjectiles)
		proj.Update(event->World, delta_time);

	for (size_t i = m_RayProjectiles.size(); i-- > 0; )
	{
		if (m_RayProjectiles[i].ShouldDie())
		{
			std::swap(m_RayProjectiles[i], m_RayProjectiles.back());
			m_RayProjectiles.pop_back();
		}
	}

	return true;
}

void Voxel::VoxelWorld::SetCube(BlockCoord coords, std::unique_ptr<ICube> cube)
{
	std::unique_lock lock(m_ChunksMutex);
	auto it = m_Chunks.find(coords.Chunk);
	if (it != m_Chunks.end() && it->second)
	{
		it->second->set(coords.Block.x, coords.Block.y, coords.Block.z, std::move(cube));
	}
	else
	{
		m_UpdateBlockChanges[coords.Chunk].emplace_back(std::make_pair(coords.Block, std::move(cube)));
	}
}

void Voxel::VoxelWorld::SetCube(BlockCoord coord, const Voxel::SerialBlock& block)
{
	std::unique_lock lock(m_ChunksMutex);
	auto it = m_Chunks.find(coord.Chunk);
	if (it != m_Chunks.end() && it->second)
	{
		it->second->set(coord.Block, block);
	}
	else
	{
		m_BlockChanges[coord.Chunk].emplace_back(std::make_pair(coord.Block, block));
	}
}

void Voxel::VoxelWorld::SetCube(BlockCoord coord, const NamedBlock& block)
{
	return SetCube(coord, SerialBlock{ VoxelStore::Instance().GetIDFor(block.Name), block.Data });
}

Voxel::ICube* Voxel::VoxelWorld::GetCubeAt(BlockCoord coord)
{
	std::shared_lock lock(m_ChunksMutex);
	auto it = m_Chunks.find(coord.Chunk);
	if (it != m_Chunks.end() && it->second)
	{
		return it->second->get(coord.Block.x, coord.Block.y, coord.Block.z);
	}
	return nullptr;
}

const Voxel::ICube* Voxel::VoxelWorld::GetCubeAt(BlockCoord coord) const
{
	std::shared_lock lock(m_ChunksMutex);
	auto it = m_Chunks.find(coord.Chunk);
	if (it != m_Chunks.end() && it->second)
		return it->second->get(coord.Block.x, coord.Block.y, coord.Block.z);
	return nullptr;
}

Voxel::SerialBlock Voxel::VoxelWorld::GetCubeDataAt(BlockCoord coord) const
{
	std::shared_lock lock(m_ChunksMutex);
	auto it = m_Chunks.find(coord.Chunk);
	if (it != m_Chunks.end() && it->second)
	{
		return it->second->get_data(coord.Block);
	}
	return VoxelStore::EmptyBlockData;
}

bool Voxel::VoxelWorld::IsCubeAt(BlockCoord coord) const
{
	return GetCubeAt(coord) != nullptr;
}

Voxel::BlockCoord Voxel::VoxelWorld::GetBlockCoordFromPhys(floaty3 phys_pos) const
{
	BlockCoord out;

	double x = (double)phys_pos.x + (double)m_PhysicsDisplacement.x, y = (double)phys_pos.y + (double)m_PhysicsDisplacement.y, z = (double)phys_pos.z + (double)m_PhysicsDisplacement.z;

	double c_x = x / Voxel::Chunk_Width_Double, c_y = y / Voxel::Chunk_Tallness_Double, c_z = z / Voxel::Chunk_Width_Double;
	double c_x_f = floor(c_x), c_y_f = floor(c_y), c_z_f = floor(c_z);

	out.Chunk.X = (int64_t)c_x_f;
	out.Chunk.Y = (int64_t)c_y_f;
	out.Chunk.Z = (int64_t)c_z_f;

	double floor_dif_x = c_x - c_x_f, floor_dif_y = c_y - c_y_f, floor_dif_z = c_z - c_z_f;

	out.Block.x = (uint32_t)std::floor(floor_dif_x * (double)Voxel::Chunk_Size);
	out.Block.y = (uint32_t)std::floor(floor_dif_y * (double)Voxel::Chunk_Height);
	out.Block.z = (uint32_t)std::floor(floor_dif_z * (double)Voxel::Chunk_Size);
	//out.X = std::max(std::min(out.X, Chunk_Size - 1), 0ull);
	//out.Y = std::max(std::min(out.Y, Chunk_Height - 1), 0ull);
	//out.Z = std::max(std::min(out.Z, Chunk_Size - 1), 0ull);

	return out;
}

Voxel::ChunkCoord Voxel::VoxelWorld::GetChunkCoordFromPhys(floaty3 phys_pos) const
{
	ChunkCoord out;

	double x = (double)phys_pos.x + (double)m_PhysicsDisplacement.x, y = (double)phys_pos.y + (double)m_PhysicsDisplacement.y, z = (double)phys_pos.z + (double)m_PhysicsDisplacement.z;

	out.X = (int64_t)floor(x / Voxel::Chunk_Width_Double);
	out.Y = (int64_t)floor(y / Voxel::Chunk_Tallness_Double);
	out.Z = (int64_t)floor(z / Voxel::Chunk_Width_Double);

	return out;
}

floaty3 Voxel::VoxelWorld::GetPhysPosFromBlockCoord(Voxel::BlockCoord coord) const
{
	floaty3 out;

	out.x = (float)((double)coord.Chunk.X * Voxel::Chunk_Width_Double + (double)coord.Block.x * (double)Voxel::BlockSize);
	out.y = (float)((double)coord.Chunk.Y * Voxel::Chunk_Tallness_Double + (double)coord.Block.y * (double)Voxel::BlockSize);
	out.z = (float)((double)coord.Chunk.Z * Voxel::Chunk_Width_Double + (double)coord.Block.z * (double)Voxel::BlockSize);

	out += m_PhysicsDisplacement;

	out.x += (BlockSize * 0.5f);
	out.y += (BlockSize * 0.5f);
	out.z += (BlockSize * 0.5f);

	return out;
}

floaty3 Voxel::VoxelWorld::GetPhysPosOfBlock(Voxel::ICube* cube) const
{
	if (!cube)
		return { 0.f, 0.f, 0.f };
	return GetPhysPosFromBlockCoord(cube->GetWorldPos());
}

void Voxel::VoxelWorld::AddEntity(std::unique_ptr<Entity> entity)
{
	m_DynamicEntities.emplace(entity.get(), std::move(entity));
}

void Voxel::VoxelWorld::RemoveEntity(Entity *entity)
{
	this->m_ToRemoveEntities.emplace_back(entity);
}

void Voxel::VoxelWorld::UnloadChunk(std::unique_ptr<VoxelChunk> chunk)
{
	if (!chunk)
		return;
	// Rudimentary diff saving
	auto chunkPos = chunk->GetCoord();
	//DINFO("Unloading chunk (" + std::to_string(chunkPos.X) + ", " + std::to_string(chunkPos.Y) + ", " + std::to_string(chunkPos.Z) + ")");
	auto generatedData = (m_Stuff.m_ChunkLoader ? m_Stuff.m_ChunkLoader->LoadChunk(chunkPos) : Voxel::RawChunkDataMap{});

	for (uint8_t x = 0; x < Chunk_Size; ++x)
	{
		for (uint8_t y = 0; y < Chunk_Height; ++y)
		{
			for (uint8_t z = 0; z < Chunk_Size; ++z)
			{
				auto key = ChunkBlockCoord{ x, y, z };
				auto it = generatedData.find(key);
				if (it == generatedData.end())
				{
					if (chunk->get(x, y, z) != nullptr)
					{
						m_UpdateBlockChanges[chunkPos].emplace_back(std::make_pair(key, std::move(chunk->take(key))));
					}
					else if (chunk->get_data(key).ID != 0)
					{
						m_BlockChanges[chunkPos].emplace_back(std::make_pair(key, chunk->get_data(key)));
					}
				}
				else if (chunk->get_data(key) != it->second)
				{
					m_BlockChanges[chunkPos].emplace_back(std::make_pair(key, chunk->get_data(key)));
				}
			}
		}
	}

	chunk = nullptr;
}

GLuint Voxel::VoxelWorld::CreateVAO()
{
	GLuint out;
	glGenVertexArrays(1, &out);
	return out;
}

GLuint Voxel::VoxelWorld::CreateVBO()
{
	constexpr float Girth = 0.1f;
	constexpr float BaseLength = 1.f;
	constexpr floaty3 BaseVertices[8] = { { -Girth, 0.f, 0.f }, { 0.f, -Girth, 0.f }, { Girth, 0.f, 0.f }, { 0.f, Girth, 0.f },   { -Girth, 0.f, -BaseLength }, { 0.f, -Girth, -BaseLength }, { Girth, 0.f, -BaseLength }, { 0.f, Girth, -BaseLength } };

	GLuint buf;
	glGenBuffers(1, &buf);
	glBindBuffer(GL_ARRAY_BUFFER, buf);
	glBufferData(GL_ARRAY_BUFFER, sizeof(BaseVertices) / sizeof(floaty3), BaseVertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	return buf;
}

GLuint Voxel::VoxelWorld::CreateIBO()
{
	constexpr GLuint Indices[14] = { 2, 3, 6, 7, 4, 3, 0, 0, 0, 0, 0, 0, 0, 0  };

	GLuint buf;
	
	glGenBuffers(1, &buf);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buf);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Indices) / sizeof(Indices[0]), Indices, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	return buf;
}

void Voxel::VoxelWorld::UpdateInstanceStuff(const std::vector<ProjInstanceData> &data)
{
	CHECK_GL_ERR("Before Update Of Instance Stuffs");
	if (data.size() > m_InstanceBufferSize)
	{
		m_InstanceBufferSize = data.size();

		glBindVertexArray(m_ProjectileVAO.Get());
		
		GLuint newbuf = 0u;
		glGenBuffers(1, &newbuf);
		glBindBuffer(GL_ARRAY_BUFFER, newbuf);
		glBufferData(GL_ARRAY_BUFFER, sizeof(ProjInstanceData) * data.size(), data.data(), GL_DYNAMIC_DRAW);

		glEnableVertexAttribArray(3);
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Matrixy3x3), (GLvoid *)offsetof(Matrixy3x3, m11));
		glEnableVertexAttribArray(4);
		glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Matrixy3x3), (GLvoid *)offsetof(Matrixy3x3, m12));
		glEnableVertexAttribArray(5);
		glVertexAttribPointer(5, 3, GL_FLOAT, GL_FALSE, sizeof(Matrixy3x3), (GLvoid *)offsetof(Matrixy3x3, m13));

		glVertexAttribDivisor(3, 1);
		glVertexAttribDivisor(4, 1);
		glVertexAttribDivisor(5, 1);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		m_ProjectileInstanceBuf.Reset(newbuf);
		glBindVertexArray(0);
	}
	else
	{
		glBindBuffer(GL_ARRAY_BUFFER, m_ProjectileInstanceBuf.Get());
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Matrixy3x3) * data.size(), data.data());
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}
	CHECK_GL_ERR("Done Instance Buffer Updating");
}

void Voxel::VoxelWorld::ApplyChunkChanges()
{
	static std::vector<ChunkCoord> toRemove = {};
	toRemove.clear();
	for (auto& chunk_change_pair : m_BlockChanges)
	{
		auto& chunk = chunk_change_pair.first;
		auto it = m_Chunks.find(chunk);
		if (it == m_Chunks.end() || !it->second)
			continue;

		auto& changes = chunk_change_pair.second;
		for (auto& change : changes)
		{
			it->second->set(change.first, std::move(change.second));
		}
		changes.clear();
		toRemove.push_back(chunk);
	}

	for (auto& chunk : toRemove)
	{
		m_BlockChanges.erase(chunk);
	}
	toRemove.clear();
}

void Voxel::VoxelWorld::CheckLoadingThread()
{
	std::unique_ptr<Voxel::LoadedChunk> chunkDat;
	while (m_LoadingStuff->Loaded.try_pop(chunkDat))
	{
		if (!chunkDat)
			continue;
		{
			std::shared_lock lock(m_ChunksMutex);
			auto it = m_Chunks.find(chunkDat->Coord);

			// Skip if not an expected chunk (expected chunks are put into m_Chunks as nullptrs)
			if (it == m_Chunks.end())
				continue;

			// Skip if there's already a chunk there
			if (it->second)
				continue;
		}

		std::unique_lock lock(m_ChunksMutex);
		auto coord = chunkDat->Coord;
		auto chunkIt = m_Chunks.insert_or_assign(coord, std::make_unique<VoxelChunk>(GetContainer(), mResources, this, ChunkOrigin(coord), std::move(chunkDat)));
		auto& chunk = chunkIt.first->second;
		{
			auto it = m_BlockChanges.find(coord);
			if (it != m_BlockChanges.end())
			{
				// Apply chunk changes
				for (auto& change : it->second)
				{
					chunk->set(change.first, std::move(change.second));
				}
				m_BlockChanges.erase(it);
			}
		}
		{
			auto it = m_UpdateBlockChanges.find(coord);
			if (it != m_UpdateBlockChanges.end())
			{
				for (auto& change : it->second)
				{
					chunk->set(change.first, std::move(change.second));
				}
				m_UpdateBlockChanges.erase(it);
			}
		}
	}
}

void Voxel::VoxelWorld::Load(ChunkCoord at)
{
	PROFILE_PUSH_AGG("Load Chunk");
	PROFILE_PUSH("Chunk Find");
	std::shared_lock lock(m_ChunksMutex);
	auto it = m_Chunks.find(at);
	PROFILE_POP();
	if (it == m_Chunks.end())
	{
		// Multi threaded:
		// TODO: Add a chunk to loading thread's queue and mark it as 'being loaded' so it isn't re-added
		lock.unlock();
		{
			std::unique_lock write_lock(m_ChunksMutex);
			m_Chunks.emplace(std::make_pair(at, nullptr));
		}
		m_LoadingStuff->ToLoad.push(at);
		PROFILE_PUSH("Chunk Emplace");
		//m_Chunks.emplace(at, std::make_unique<VoxelChunk>(GetContainer(), GetResources(), this, ChunkOrigin(at), m_Stuff.m_ChunkLoader->LoadChunk(at.X, at.Y, at.Z), at)); // Single threaded
		PROFILE_POP();
	}
	PROFILE_POP();
}

void Voxel::VoxelWorld::Unload(ChunkCoord at)
{
	std::unique_lock lock(m_ChunksMutex);
	if (auto it = m_Chunks.find(at); it != m_Chunks.end())
	{
		// TODO: write unloading crap
		std::unique_ptr<VoxelChunk> tmp = std::move(it->second);
		m_Chunks.erase(it);
		m_Stuff.m_ChunkUnloader->UnloadChunk(std::move(tmp));
	}
}

floaty3 Voxel::VoxelWorld::ChunkOrigin(ChunkCoord of)
{
	return 
	{
		Chunk_Width    * (float)of.X,
		Chunk_Tallness * (float)of.Y,
		Chunk_Width    * (float)of.Z
	};
}

void Voxel::VoxelWorld::DoRemoveEntities()
{
	for (auto &entity : m_ToRemoveEntities)
	{
		auto it = m_DynamicEntities.find(entity);
		if (it != m_DynamicEntities.end())
		{
			m_DynamicEntities.erase(it);
		}
	}
	m_ToRemoveEntities.clear();
}


template<>
void Voxel::VoxelWorld::AddProjectile<Voxel::RayProjectile, floaty3, floaty3, float, Voxel::Entity *, Voxel::DamageDescription>(floaty3 vel, floaty3 pos, float distance, Voxel::Entity *source, Voxel::DamageDescription dam)
{
	m_RayProjectiles.emplace_back(vel, pos, distance, source, dam);
}

template<>
void Voxel::VoxelWorld::AddProjectile<Voxel::HitScanProjectile, floaty3, floaty3, Voxel::Entity *, Voxel::DamageDescription>(floaty3 direction, floaty3 start, Voxel::Entity *source, Voxel::DamageDescription dam)
{
	m_HitscanProjectiles.emplace_back(direction, start, source, dam);
}

void Voxel::DoChunkLoading(std::shared_ptr<LoadingStuff> stuff, LoadingOtherStuff other)
{
	using namespace std::chrono;
	while (!stuff->QuitVal.load())
	{
		if (Voxel::ChunkCoord toLoad; stuff->ToLoad.try_pop(toLoad, 10ms))
		{
			auto data = other.GetChunkDataFunc(toLoad);

			auto loaded = Voxel::GenerateChunkMesh(*data, toLoad, other.GetBlockIdFunc);
			loaded->ChunkDat = std::move(*data);

			stuff->Loaded.push(std::move(loaded));
		}
	}
}
