#include "VoxelWorld.h"

#include "Helpers/MathHelper.h"
#include "Helpers/ProfileHelper.h"

#include "Systems/Events/PhysicsEvent.h"

#include "VoxelAbility.h"

Voxel::VoxelWorld::VoxelWorld(G1::IGSpace *container, CommonResources *resources, WorldStuff stuff) : IShape(container, "Voxel World"), FullResourceHolder(resources), m_Stuff(stuff)
{
	m_Stuff.ChunkLeniance = Math::min<size_t>()(m_Stuff.ChunkLeniance, m_Stuff.HalfBonusWidth);
	m_Stuff.ChunkLeniance = Math::min<size_t>()(m_Stuff.ChunkLeniance, m_Stuff.HalfBonusHeight);
	m_Stuff.ChunkLeniance = Math::min<size_t>()(m_Stuff.ChunkLeniance, m_Stuff.HalfBonusDepth);

	if (m_Stuff.ChunkLeniance < 1)
		m_Stuff.ChunkLeniance = 1;
}

Voxel::VoxelWorld::~VoxelWorld()
{
}

floaty3 Voxel::VoxelWorld::Update(floaty3 New_Centre)
{
	PROFILE_PUSH("VoxelWorld Update");
	// What to do:
	// check if player if ChunkLeniance chunks away from the centre

	// Convert New_Centre to World space
	DOUBLE3 centre = (DOUBLE3)New_Centre + m_PhysicsDisplacement;

	// Convert World space to chunk space
	centre = { centre.x / Chunk_Width_Double, centre.y / Chunk_Tallness_Double, centre.z / Chunk_Width_Double };

	// centre is now in world chunk space
	ChunkCoord chunk_centre = GetChunkCoordFromPhys(New_Centre);


	// Now create the desired cube of chunks around the centre

	int64_t lower_x = (chunk_centre.X - (int64_t)m_Stuff.HalfBonusWidth), upper_x = (chunk_centre.X + (int64_t)m_Stuff.HalfBonusWidth);
	int64_t lower_y = (chunk_centre.Y - (int64_t)m_Stuff.HalfBonusHeight), upper_y = (chunk_centre.Y + (int64_t)m_Stuff.HalfBonusHeight);
	int64_t lower_z = (chunk_centre.Z - (int64_t)m_Stuff.HalfBonusDepth), upper_z = (chunk_centre.Z + (int64_t)m_Stuff.HalfBonusDepth);

	// Unload Chunks
	std::vector<ChunkCoord> to_unload;

	for (auto &chunk : m_Chunks)
	{
		if (chunk.first.X < lower_x || chunk.first.X > upper_x ||
			chunk.first.Y < lower_y || chunk.first.Y > upper_y ||
			chunk.first.Z < lower_z || chunk.first.Z > upper_z)
		{
			to_unload.emplace_back(chunk.first);
		}
	}

	for (auto &chunk : to_unload)
	{
		std::unique_ptr<ChunkyBoi> ptr = std::move(m_Chunks[chunk]);
		m_Stuff.m_ChunkUnloader->UnloadChunk(std::move(ptr));
		m_Chunks.erase(chunk);
	}

	// Load new chunks

	// Convert to centre + half extents
	const int64_t &centre_x = chunk_centre.X;
	const int64_t &centre_y = chunk_centre.Y;
	const int64_t &centre_z = chunk_centre.Z;

	/*for (int64_t i = 0; i <= (int64_t)m_Stuff.HalfBonusWidth; ++i)
	{
		for (int64_t j = 0; j <= (int64_t)m_Stuff.HalfBonusHeight; ++j)
		{
			for (int64_t n = 0; n <= (int64_t)m_Stuff.HalfBonusDepth; ++n)
			{
				ChunkCoord point1 = { centre_x + i, centre_y + j, centre_z + n };
				ChunkCoord point2 = { centre_x - i, centre_y - j, centre_z - n };
				Load(point1);
				Load(point2);
			}
		}
	}*/

	int64_t x = 0, y = 0, z = 0;

	bool go = true, do_x = false, do_y = false, do_z = false;

	PROFILE_PUSH("Loading Chunks");
	Load({ centre_x, centre_y, centre_z });

	while (true)
	{
		go = false;
		if (x < static_cast<int64_t>(m_Stuff.HalfBonusWidth))
		{
			++x;
			do_x = go = true;
		}
		if (y < static_cast<int64_t>(m_Stuff.HalfBonusHeight))
		{
			++y;
			do_y = go = true;
		}
		if (z < static_cast<int64_t>(m_Stuff.HalfBonusDepth))
		{
			++z;
			do_z = go = true;
		}
		if (!go)
			break;
		

		// X ---------------------------------------------------------------------
		if (do_x)
		{
			// Load positive and negative versions at the same time
			int x_y = 0, x_z = 0;
			int y_extent = (int)(x < static_cast<int64_t>(m_Stuff.HalfBonusHeight) ? x : m_Stuff.HalfBonusHeight);
			int z_extent = (int)(x < static_cast<int64_t>(m_Stuff.HalfBonusDepth) ? x : m_Stuff.HalfBonusDepth);
			bool x_do_y = false, x_do_z = false, x_go = false;

			Load({ centre_x + x, centre_y, centre_z });
			Load({ centre_x - x, centre_y, centre_z });

			while (true)
			{
				x_go = false;
				if (x_y < y_extent)
				{
					++x_y;
					x_do_y = true;
					x_go = true;
				}
				if (x_z < z_extent)
				{
					++x_z;
					x_do_z = true;
					x_go = true;
				}

				if (!x_go)
					break;

				// Now we have a x, and a z value to work with,
				// Now 'draw' the vertical lines
				// Z first just because chunks are taller than wide
				if (x_do_z)
				{
					for (int i = 0; i <= x_y; ++i)
					{
						Load({ centre_x + x, centre_y + i, centre_z + x_z });
						Load({ centre_x + x, centre_y + i, centre_z - x_z });
						Load({ centre_x + x, centre_y - i, centre_z + x_z });
						Load({ centre_x + x, centre_y - i, centre_z - x_z });
						Load({ centre_x - x, centre_y + i, centre_z + x_z });
						Load({ centre_x - x, centre_y + i, centre_z - x_z });
						Load({ centre_x - x, centre_y - i, centre_z + x_z });
						Load({ centre_x - x, centre_y - i, centre_z - x_z });
					}
				}

				// Now with the x and y values 'draw' the horizontal lines
				if (x_do_y)
				{
					for (int i = 0; i <= x_z; ++i)
					{
						Load({ centre_x + x, centre_y + x_y, centre_z + i });
						Load({ centre_x + x, centre_y + x_y, centre_z - i });
						Load({ centre_x + x, centre_y - x_y, centre_z + i });
						Load({ centre_x + x, centre_y - x_y, centre_z - i });
						Load({ centre_x - x, centre_y + x_y, centre_z + i });
						Load({ centre_x - x, centre_y + x_y, centre_z - i });
						Load({ centre_x - x, centre_y - x_y, centre_z + i });
						Load({ centre_x - x, centre_y - x_y, centre_z - i });
					}
				}
			}

			

		}

		// Y----------------------------------------------------------------------
		if (do_y)
		{
			// Load positive and negative versions at the same time
			int y_x = 0, y_z = 0;
			int x_extent = (int)(y < (int64_t)m_Stuff.HalfBonusWidth ? y : m_Stuff.HalfBonusWidth);
			int z_extent = (int)(y < (int64_t)m_Stuff.HalfBonusDepth ? y : m_Stuff.HalfBonusDepth);
			bool y_do_x = false, y_do_z = false, y_go = false;

			Load({ centre_x , centre_y + y, centre_z });
			Load({ centre_x , centre_y - y, centre_z });

			while (true)
			{
				y_go = false;
				if (y_x < x_extent)
				{
					++y_x;
					y_do_x = true;
					y_go = true;
				}
				if (y_z < z_extent)
				{
					++y_z;
					y_do_z = true;
					y_go = true;
				}

				if (!y_go)
					break;

				// 'Drawing' the lines with the established 2 variables
				if (y_do_z)
				{
					for (int i = 0; i <= y_z; ++i)
					{
						Load({ centre_x + y_x, centre_y + y, centre_z + i });
						Load({ centre_x + y_x, centre_y + y, centre_z - i });
						Load({ centre_x + y_x, centre_y - y, centre_z + i });
						Load({ centre_x + y_x, centre_y - y, centre_z - i });
						Load({ centre_x - y_x, centre_y + y, centre_z + i });
						Load({ centre_x - y_x, centre_y + y, centre_z - i });
						Load({ centre_x - y_x, centre_y - y, centre_z + i });
						Load({ centre_x - y_x, centre_y - y, centre_z - i });
					}
				}

				if (y_do_z)
				{
					for (int i = 0; i <= y_x; ++i)
					{
						Load({ centre_x + i, centre_y + y, centre_z + y_z });
						Load({ centre_x + i, centre_y + y, centre_z - y_z });
						Load({ centre_x + i, centre_y - y, centre_z + y_z });
						Load({ centre_x + i, centre_y - y, centre_z - y_z });
						Load({ centre_x - i, centre_y + y, centre_z + y_z });
						Load({ centre_x - i, centre_y + y, centre_z - y_z });
						Load({ centre_x - i, centre_y - y, centre_z + y_z });
						Load({ centre_x - i, centre_y - y, centre_z - y_z });
					}
				}
			}
		}

		// Z--------------------------------------------------------------
		if (do_z)
		{
			// Load positive and negative versions at the same time
			int z_x = 0, z_y = 0;
			int x_extent = (int)(z < m_Stuff.HalfBonusWidth ? z : m_Stuff.HalfBonusWidth);
			int y_extent = (int)(z < m_Stuff.HalfBonusHeight ? z : m_Stuff.HalfBonusHeight);
			bool z_do_x = false, z_do_y = false, z_go = false;

			Load({ centre_x , centre_y, centre_z + z });
			Load({ centre_x , centre_y, centre_z - z });

			while (true)
			{
				z_go = false;
				if (z_x < x_extent)
				{
					++z_x;
					z_do_x = true;
					z_go = true;
				}
				if (z_y < y_extent)
				{
					++z_y;
					z_do_y = true;
					z_go = true;
				}

				if (!z_go)
					break;

				// 'Draw' the lines
				if (z_do_x)
				{
					for (int i = 0; i <= z_y; ++i)
					{
						Load({ centre_x + z_x, centre_y + i, centre_z + z });
						Load({ centre_x + z_x, centre_y + i, centre_z - z });
						Load({ centre_x + z_x, centre_y - i, centre_z + z });
						Load({ centre_x + z_x, centre_y - i, centre_z - z });
						Load({ centre_x - z_x, centre_y + i, centre_z + z });
						Load({ centre_x - z_x, centre_y + i, centre_z - z });
						Load({ centre_x - z_x, centre_y - i, centre_z + z });
						Load({ centre_x - z_x, centre_y - i, centre_z - z });
					}
				}

				if (z_do_y)
				{
					for (int i = 0; i <= z_x; ++i)
					{
						Load({ centre_x + i, centre_y + z_y, centre_z + z });
						Load({ centre_x + i, centre_y + z_y, centre_z - z });
						Load({ centre_x + i, centre_y - z_y, centre_z + z });
						Load({ centre_x + i, centre_y - z_y, centre_z - z });
						Load({ centre_x - i, centre_y + z_y, centre_z + z });
						Load({ centre_x - i, centre_y + z_y, centre_z - z });
						Load({ centre_x - i, centre_y - z_y, centre_z + z });
						Load({ centre_x - i, centre_y - z_y, centre_z - z });
					}
				}
			}
		}
	}

	PROFILE_POP();

	PROFILE_POP();
	return { 0.f, 0.f, 0.f };
}

void Voxel::VoxelWorld::BeforeDraw()
{
	PROFILE_PUSH("VoxelWorld BeforeDraw");
	PROFILE_PUSH("Chunks");
	for (auto &chunk : m_Chunks)
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
	PROFILE_PUSH("Chunks");
	for (auto &chunk : m_Chunks)
		chunk.second->AfterDraw();
	PROFILE_POP();
	PROFILE_PUSH("Entities");
	for (auto &entity : m_DynamicEntities)
		entity.second->AfterDraw();
	PROFILE_POP();
	PROFILE_POP();

	DoRemoveEntities();
}

bool Voxel::VoxelWorld::Receive(Events::IEvent *e)
{

	return true;
}

bool Voxel::VoxelWorld::Receive(Event::AfterPhysicsEvent *event)
{
	double delta_time = *mResources->DeltaTime;

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

void Voxel::VoxelWorld::ReplaceStaticCube(BlockCoord coords, std::unique_ptr<ICube> cube)
{
	auto it = m_Chunks.find(coords.Chunk);
	if (it != m_Chunks.end())
	{
		it->second->set(coords.X, coords.Y, coords.Z, std::move(cube));
	}
}

void Voxel::VoxelWorld::SetStaticCube(BlockCoord coord)
{
	auto it = m_Chunks.find(coord.Chunk);
	if (it != m_Chunks.end())
	{
		it->second->create(coord.X, coord.Y, coord.Z);
	}
}

Voxel::ICube* Voxel::VoxelWorld::GetCubeAt(BlockCoord coord)
{
	auto it = m_Chunks.find(coord.Chunk);
	if (it != m_Chunks.end())
	{
		return it->second->get(coord.X, coord.Y, coord.Z);
	}
	return nullptr;
}

bool Voxel::VoxelWorld::IsCubeAt(BlockCoord coord)
{
	return GetCubeAt(coord) != nullptr;
}

Voxel::BlockCoord Voxel::VoxelWorld::GetBlockCoordFromPhys(floaty3 phys_pos)
{
	BlockCoord out;

	double x = (double)phys_pos.x + (double)m_PhysicsDisplacement.x, y = (double)phys_pos.y + (double)m_PhysicsDisplacement.y, z = (double)phys_pos.z + (double)m_PhysicsDisplacement.z;

	double c_x = x / Voxel::Chunk_Width_Double, c_y = y / Voxel::Chunk_Tallness_Double, c_z = z / Voxel::Chunk_Width_Double;
	double c_x_f = floor(c_x), c_y_f = floor(c_y), c_z_f = floor(c_z);

	out.Chunk.X = (int64_t)c_x_f;
	out.Chunk.Y = (int64_t)c_y_f;
	out.Chunk.Z = (int64_t)c_z_f;

	double floor_dif_x = c_x - c_x_f, floor_dif_y = c_y - c_y_f, floor_dif_z = c_z - c_z_f;

	out.X = (size_t)std::floor(floor_dif_x * (double)Voxel::Chunk_Size);
	out.Y = (size_t)std::floor(floor_dif_y * (double)Voxel::Chunk_Height);
	out.Z = (size_t)std::floor(floor_dif_z * (double)Voxel::Chunk_Size);
	//out.X = std::max(std::min(out.X, Chunk_Size - 1), 0ull);
	//out.Y = std::max(std::min(out.Y, Chunk_Height - 1), 0ull);
	//out.Z = std::max(std::min(out.Z, Chunk_Size - 1), 0ull);

	return out;
}

Voxel::ChunkCoord Voxel::VoxelWorld::GetChunkCoordFromPhys(floaty3 phys_pos)
{
	ChunkCoord out;

	double x = (double)phys_pos.x + (double)m_PhysicsDisplacement.x, y = (double)phys_pos.y + (double)m_PhysicsDisplacement.y, z = (double)phys_pos.z + (double)m_PhysicsDisplacement.z;

	out.X = (int64_t)floor(x / Voxel::Chunk_Width_Double);
	out.Y = (int64_t)floor(y / Voxel::Chunk_Tallness_Double);
	out.Z = (int64_t)floor(z / Voxel::Chunk_Width_Double);

	return out;
}

void Voxel::VoxelWorld::AddEntity(std::unique_ptr<Entity> entity)
{
	m_DynamicEntities.emplace(entity.get(), std::move(entity));
}

void Voxel::VoxelWorld::RemoveEntity(Entity *entity)
{
	this->m_ToRemoveEntities.emplace_back(entity);
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

void Voxel::VoxelWorld::Load(ChunkCoord at)
{
	auto it = m_Chunks.find(at);
	if (it == m_Chunks.end())
	{
		m_Chunks.emplace(at, std::make_unique<ChunkyBoi>(GetContainer(), GetResources(), this, ChunkOrigin(at), m_Stuff.m_ChunkLoader->LoadChunk(at.X, at.Y, at.Z), at));
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