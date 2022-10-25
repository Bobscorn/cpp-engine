#include "VoxelChunk.h"

#include "Helpers/MeshHelper.h"
#include "Helpers/ProfileHelper.h"

#include "VoxelWorld.h"
#include "Drawing/VoxelStore.h"
#include "Drawing/IRen3Dv2.h"

#include "Structure/BulletBitmasks.h"

#include <algorithm>
#include <cstring>

constexpr floaty3 OriginFromCoord(Voxel::ChunkCoord coord)
{
	return { ((float)Voxel::Chunk_Size * -0.5f) - 0.5f + (float)((int64_t)Voxel::Chunk_Size * coord.X), ((float)Voxel::Chunk_Size * -0.5f) - 0.5f + (float)((int64_t)Voxel::Chunk_Size * coord.Y), ((float)Voxel::Chunk_Size * -0.5f) - 0.5f + (float)((int64_t)Voxel::Chunk_Size * coord.Z) };
}

constexpr floaty3 PositionFromCoord(floaty3 origin, size_t x, size_t y, size_t z)
{
	return { origin.x + ((float)x + 0.5f) * Voxel::BlockSize, origin.y + ((float)y + 0.5f) * Voxel::BlockSize, origin.z + ((float)z + 0.5f) * Voxel::BlockSize };
}


namespace Voxel
{

	std::unique_ptr<Voxel::ChunkData> ConvertMapToData(const Voxel::RawChunkDataMap& map)
	{
		auto data = std::make_unique<Voxel::ChunkData>();

		for (auto& b : map)
		{
			auto& key = b.first;
			(*data)[key.x][key.y][key.z] = b.second;
		}

		return data;
	}

	Voxel::ChunkyBoi::ChunkyBoi(G1::IGSpace* container, CommonResources* resources, VoxelWorld* world, floaty3 origin, ChunkCoord coord)
		: G1::IShape(container, CreateChunkName(coord))
		, FullResourceHolder(resources)
		, m_World(world)
		, m_Culler(CreateCuller(m_Origin))
		, m_Origin(origin)
		, m_Coord(coord)
		, m_Data()
	{
	}

	Voxel::ChunkyBoi::ChunkyBoi(G1::IGSpace* container, CommonResources* resources, VoxelWorld* world, floaty3 origin, RawChunkDataMap initial_dat, ChunkCoord coord)
		: G1::IShape(container, CreateChunkName(coord))
		, FullResourceHolder(resources)
		, m_World(world)
		, m_Culler(CreateCuller(m_Origin))
		, m_Origin(origin)
		, m_Data()
		, m_Material(Drawing::MaterialStore::Instance().GetMaterial("default-voxel-material"))
		, m_Coord(coord)
		, m_Mesh(std::make_shared<Drawing::Mesh>())
	{
		/*PROFILE_PUSH("Chunk Creating Blocks");
		for (auto& pair : initial_dat)
		{
			if (pair.second.ID)
			{
				create(pair.first.X, pair.first.Y, pair.first.Z);
			}
		}
		PROFILE_POP();*/
		auto p = ConvertMapToData(initial_dat);
		SetFrom(GenerateChunkMesh(*p, coord, [world = this->m_World](BlockCoord coord) { return world->GetCubeDataAt(coord); }));
		PROFILE_PUSH("Submitting DrawCall");
		auto name = std::string("Chunk (") + std::to_string(coord.X) + ", " + std::to_string(coord.Y) + ", " + std::to_string(coord.Z) + ")";
		m_DrawCall = resources->Ren3v2->SubmitDrawCall(Drawing::DrawCallv2{ m_Mesh, m_Material, std::make_shared<Matrixy4x4>(Matrixy4x4::Translate(m_Origin)), name, true });
		PROFILE_POP();
	}

	ChunkyBoi::ChunkyBoi(G1::IGSpace* container, CommonResources* resources, VoxelWorld* world, floaty3 origin, std::unique_ptr<LoadedChunk> preloadedStuff)
		: G1::IShape(container, CreateChunkName(preloadedStuff->Coord))
		, FullResourceHolder(resources)
		, m_World(world)
		, m_Culler(CreateCuller(m_Origin))
		, m_Origin(origin)
		, m_Data()
		, m_Material(Drawing::MaterialStore::Instance().GetMaterial("default-voxel-material"))
		, m_Coord(preloadedStuff->Coord)
		, m_Mesh()
	{
		SetFrom(std::move(preloadedStuff));
		PROFILE_PUSH("Submitting DrawCall");
		
		m_DrawCall = resources->Ren3v2->SubmitDrawCall(Drawing::DrawCallv2{ m_Mesh, m_Material, std::make_shared<Matrixy4x4>(Matrixy4x4::Translate(m_Origin)), CreateChunkName(m_Coord), true});
		PROFILE_POP();
	}

	Voxel::ChunkyBoi::~ChunkyBoi()
	{
		if (m_Body)
		{
			Container->RequestPhysicsRemoval(m_Body.get());
		}
	}

	void Voxel::ChunkyBoi::BeforeDraw()
	{
		if (m_Culler)
			m_Culler->Flush();

		for (auto& updateBlockPair : m_UpdateBlocks)
		{
			if (updateBlockPair.second)
				updateBlockPair.second->BeforeDraw();
		}

		if (m_Dirty)
		{
			RecomputeMesh();
			m_Dirty = false;
		}
	}

	void Voxel::ChunkyBoi::AfterDraw()
	{
		for (auto& updateBlockPair : m_UpdateBlocks)
		{
			if (updateBlockPair.second)
				updateBlockPair.second->AfterDraw();
		}
	}

	void Voxel::ChunkyBoi::UpdateOrigin(floaty3 origin)
	{
		m_Origin = origin;
		m_Culler = CreateCuller(m_Origin);
		m_Dirty = true;
	}

	Voxel::ChunkyFrustumCuller* Voxel::ChunkyBoi::GetCuller() const
	{
		return m_Culler.get();
	}

	void Voxel::ChunkyBoi::set(uint32_t x, uint32_t y, uint32_t z, std::unique_ptr<Voxel::ICube> val)
	{
		ChunkBlockCoord key{ x, y, z };
		m_Data[x][y][z] = val->GetBlockData();
		m_UpdateBlocks[key] = std::move(val);
		m_Dirty = true;
	}

	void ChunkyBoi::set(ChunkBlockCoord coord, std::unique_ptr<Voxel::ICube> val)
	{
		set(coord.x, coord.y, coord.z, std::move(val));
	}

	void ChunkyBoi::set(ChunkBlockCoord coord, const SerialBlock& block)
	{
		m_Data[coord.x][coord.y][coord.z] = block;
		m_Dirty = true;
	}

	Voxel::ICube* Voxel::ChunkyBoi::get(uint32_t x, uint32_t y, uint32_t z)
	{
		auto it = m_UpdateBlocks.find(ChunkBlockCoord{ x, y, z });
		if (it != m_UpdateBlocks.end())
			return it->second.get();
		return nullptr;
	}

	ICube* ChunkyBoi::get(ChunkBlockCoord coord)
	{
		return get(coord.x, coord.y, coord.z);
	}

	SerialBlock ChunkyBoi::get_data(ChunkBlockCoord coord) const
	{
		return m_Data[coord.x][coord.y][coord.z];
	}

	ChunkCoord ChunkyBoi::GetCoord() const
	{
		return m_Coord;
	}

	std::unique_ptr<ICube> ChunkyBoi::take(ChunkBlockCoord coord)
	{
		auto it = m_UpdateBlocks.find(coord);
		if (it != m_UpdateBlocks.end())
			return std::move(it->second);
		return nullptr;
	}

	void Voxel::ChunkyBoi::SetTo(std::unique_ptr<ChunkData> data)
	{
		(void)data;
		// TODO: this method (or delete this method)
		std::unordered_map<std::string, std::shared_ptr<GeoThing>> geos;
		std::unordered_map<std::string, std::shared_ptr<Material>> mats;
		std::unordered_map<std::string, std::shared_ptr<btCollisionShape>> shapes;

		m_Dirty = true;
	}

	void ChunkyBoi::SetFrom(std::unique_ptr<LoadedChunk> preLoadedChunk, bool constructCubes)
	{
		if (constructCubes)
		{
			m_Data = std::move(preLoadedChunk->ChunkDat);
			auto& vox = Voxel::VoxelStore::Instance();
			m_UpdateBlocks.clear();
			for (unsigned int x = 0; x < Chunk_Size; ++x)
			{
				for (unsigned int y = 0; y < Chunk_Height; ++y)
				{
					for (unsigned int z = 0; z < Chunk_Size; ++z)
					{
						auto& block = m_Data[x][y][z];
						const VoxelBlock* desc;
						if (vox.TryGetDescription(block.ID, desc) && desc->WantsUpdate)
						{
							auto tmp = vox.CreateCube(preLoadedChunk->ChunkDat[x][y][z]);
							if (tmp)
							{
								m_UpdateBlocks[ChunkBlockCoord{ x,y,z }] = std::move(tmp);
							}
						}
					}
				}
			}
		}

		auto m = Drawing::Mesh{ std::move(preLoadedChunk->Mesh), Drawing::MeshStorageType::DEDICATED_BUFFER };

		if (m_Mesh)
			*m_Mesh = std::move(m);
		else
			m_Mesh = std::make_shared<Drawing::Mesh>(std::move(m));

		if (m_Body)
			Container->RequestPhysicsRemoval(m_Body.get());
		m_Body = nullptr;

		if (preLoadedChunk->PhysicsIndices)
			m_PhysIndices.swap(*preLoadedChunk->PhysicsIndices);
		else
			m_PhysIndices.clear();
		if (preLoadedChunk->PhysicsPositions)
			m_PhysVertices.swap(*preLoadedChunk->PhysicsPositions);
		else
			m_PhysVertices.clear();

		m_MeshData = std::move(preLoadedChunk->PhysicsTriangles);
		m_Shape = std::move(preLoadedChunk->PhysicsShape);

		if (m_Shape)
		{
			m_Body = std::make_shared<btCollisionObject>();

			m_Body->setUserPointer(&this->m_Holder);
			m_Body->setCollisionShape(m_Shape.get());

			Matrixy4x4 trans = Matrixy4x4::Translate(m_Origin);
			btTransform bTrans;
			bTrans.setFromOpenGLMatrix(trans.ma);
			m_Body->setWorldTransform(bTrans);

			Container->RequestPhysicsCall(m_Body, ENVIRONMENT, PLAYER | ENTITY_GENERAL);
		}

		m_Coord = preLoadedChunk->Coord;
	}

	void Voxel::ChunkyBoi::RecomputeMesh()
	{
		if (!m_World)
		{
			DERROR("Can not recompute mesh without a voxel world!");
			return;
		}

		auto& world = *m_World;

		SetFrom(GenerateChunkMesh(m_Data, m_Coord, [&world](BlockCoord coord) { return world.GetCubeDataAt(coord); }), false);
	}

	std::string ChunkyBoi::CreateChunkName(ChunkCoord coord)
	{
		std::string name{ 0, (char)0, std::allocator<char>() };
		name.reserve(35);
		name.append("Chunk (").append(std::to_string(coord.X)).append(", ").append(std::to_string(coord.Y)).append(", ").append(std::to_string(coord.Z)).append(")");
		return name;
	}

	std::unique_ptr<ChunkyFrustumCuller> ChunkyBoi::CreateCuller(floaty3 origin)
	{
		return std::make_unique<ChunkyFrustumCuller>(origin, floaty3{ Voxel::Chunk_Size, Voxel::Chunk_Height, Voxel::Chunk_Size });
	}

	std::unique_ptr<LoadedChunk> GenerateChunkMeshT(const ChunkData& data, ChunkCoord coord, std::function<SerialBlock(BlockCoord coord)> blockDataFunc)
	{
		std::unique_ptr<Voxel::LoadedChunk> chunk = std::make_unique<Voxel::LoadedChunk>();

		chunk->Coord = coord;

		std::vector<Drawing::VoxelVertex> vertices;
		std::vector<GLuint> indices;

		// Actual generate mesh
		// v

		// Simplest method: just add 3 vertices and 3 indices for each exposed face of a block

		auto convertRelToCoord = [coord](int x, int y, int z)
		{
			BlockCoord co;
			co.Chunk = coord;
			co.Block.x = (uint32_t)x;
			co.Block.y = (uint32_t)y;
			co.Block.z = (uint32_t)z;
			if (x < 0)
			{
				--co.Chunk.X;
				co.Block.x = Chunk_Size - 1;
			}
			if (y < 0)
			{
				--co.Chunk.Y;
				co.Block.y = Chunk_Height - 1;
			}
			if (z < 0)
			{
				--co.Chunk.Z;
				co.Block.z = Chunk_Size - 1;
			}
			if (x >= (int)Chunk_Size)
			{
				++co.Chunk.X;
				co.Block.x = 0;
			}
			if (y >= (int)Chunk_Height)
			{
				++co.Chunk.Y;
				co.Block.y = 0;
			}
			if (z >= (int)Chunk_Size)
			{
				++co.Chunk.Z;
				co.Block.z = 0;
			}
			return co;
		};

		auto& vox = Voxel::VoxelStore::Instance();

		auto cubeAtHasFace = [&blockDataFunc, &data, &convertRelToCoord, &vox, coord](int chunkRelativeX, int chunkRelativeY, int chunkRelativeZ, BlockFace face)
		{
			if (chunkRelativeX < 0 || chunkRelativeY < 0 || chunkRelativeZ < 0 ||
				chunkRelativeX >= Chunk_Size || chunkRelativeY >= Chunk_Height || chunkRelativeZ >= Chunk_Size)
				return vox.GetDescOrEmpty(blockDataFunc(convertRelToCoord(chunkRelativeX, chunkRelativeY, chunkRelativeZ)).ID).FaceOpaqueness[(int)face] == FaceClosedNess::CLOSED_FACE;
			return vox.GetDescOrEmpty(data[chunkRelativeX][chunkRelativeY][chunkRelativeZ].ID).FaceOpaqueness[(int)face] == FaceClosedNess::CLOSED_FACE;
		};

		auto origin = coord;

		auto AddVerticesFunc = [&vertices, &indices, origin](int chunkRelativeX, int chunkRelativeY, int chunkRelativeZ, floaty3 direction, floaty3 tangent, FaceTexCoord coords)
		{
			auto binorm = direction.cross(tangent);

			// Generate vertices
			constexpr auto blockOffset = floaty3{ 0.5f * BlockSize, 0.5f * BlockSize, 0.5f * BlockSize }; // Offset necessary to make any origin cubes actually on the origin
			auto base = floaty3{ (float)chunkRelativeX * BlockSize, (float)chunkRelativeY * BlockSize, (float)chunkRelativeZ * BlockSize } + blockOffset;

			// Generative base vertex for copying
			Drawing::VoxelVertex vert1{};
			vert1.Position = base + direction * 0.5f * BlockSize + tangent * 0.5f * BlockSize + binorm * 0.5f * BlockSize;
			vert1.Normal = direction;
			vert1.Tangent = tangent;
			vert1.Binormal = binorm;
			vert1.TexCoord = coords.UpperTexCoord;

			Drawing::VoxelVertex vert2 = vert1;
			vert2.Position = base + direction * 0.5f * BlockSize + tangent * -0.5f * BlockSize + binorm * 0.5f * BlockSize;
			vert2.TexCoord = coords.LowerTexCoord + floaty3{ 0.f, coords.UpperTexCoord.y - coords.LowerTexCoord.y, 0.f };

			Drawing::VoxelVertex vert3 = vert1;
			vert3.Position = base + direction * 0.5f * BlockSize + tangent * -0.5f * BlockSize + binorm * -0.5f * BlockSize;
			vert3.TexCoord = coords.LowerTexCoord;

			Drawing::VoxelVertex vert4 = vert1;
			vert4.Position = base + direction * 0.5f * BlockSize + tangent * 0.5f * BlockSize + binorm * -0.5f * BlockSize;
			vert4.TexCoord = coords.LowerTexCoord + floaty3{ coords.UpperTexCoord.x - coords.LowerTexCoord.x, 0.f, 0.f };

			GLuint index1 = 0;
			GLuint index2 = 1;
			GLuint index3 = 2;

			GLuint index4 = 2;
			GLuint index5 = 3;
			GLuint index6 = 0;

			GLuint indexOffset = (GLuint)vertices.size();

			// insert indices
			indices.emplace_back(index1 + indexOffset);
			indices.emplace_back(index2 + indexOffset);
			indices.emplace_back(index3 + indexOffset);

			indices.emplace_back(index4 + indexOffset);
			indices.emplace_back(index5 + indexOffset);
			indices.emplace_back(index6 + indexOffset);

			// insert vertices
			vertices.emplace_back(vert1);
			vertices.emplace_back(vert2);
			vertices.emplace_back(vert3);
			vertices.emplace_back(vert4);
		};

		for (int x = 0; x < Chunk_Size; ++x)
		{
			for (int y = 0; y < Chunk_Height; ++y)
			{
				for (int z = 0; z < Chunk_Size; ++z)
				{
					auto& blockDat = data[x][y][z];
					if (blockDat.ID == 0)
						continue;
					const Voxel::VoxelBlock *desc;
					if (!Voxel::VoxelStore::Instance().TryGetDescription(blockDat.ID, desc))
						continue;

					// go through neighbours check if there is a block there
					// if there isn't add triangles to mesh

					// -X
					if (!cubeAtHasFace(x - 1, y, z, BlockFace::POS_X))
						AddVerticesFunc(x, y, z, floaty3{ -1.f, 0.f, 0.f }, floaty3{ 0.f, 0.f, -1.f }, desc->FaceCoordsFor(BlockFace::NEG_X));
					// +X
					if (!cubeAtHasFace(x + 1, y, z, BlockFace::NEG_X))
						AddVerticesFunc(x, y, z, floaty3{ +1.f, 0.f, 0.f }, floaty3{ 0.f, 0.f, +1.f }, desc->FaceCoordsFor(BlockFace::POS_X));
					// -Y
					if (!cubeAtHasFace(x, y - 1, z, BlockFace::POS_Y))
						AddVerticesFunc(x, y, z, floaty3{ 0.f, -1.f, 0.f }, floaty3{ +1.f, 0.f, 0.f }, desc->FaceCoordsFor(BlockFace::NEG_Y));
					// +Y
					if (!cubeAtHasFace(x, y + 1, z, BlockFace::NEG_Y))
						AddVerticesFunc(x, y, z, floaty3{ 0.f, +1.f, 0.f }, floaty3{ -1.f, 0.f, 0.f }, desc->FaceCoordsFor(BlockFace::POS_Y));
					// -Z
					if (!cubeAtHasFace(x, y, z - 1, BlockFace::POS_Z))
						AddVerticesFunc(x, y, z, floaty3{ 0.f, 0.f, -1.f }, floaty3{ +1.f, 0.f, 0.f }, desc->FaceCoordsFor(BlockFace::NEG_Z));
					// +Z
					if (!cubeAtHasFace(x, y, z + 1, BlockFace::NEG_Z))
						AddVerticesFunc(x, y, z, floaty3{ 0.f, 0.f, +1.f }, floaty3{ -1.f, 0.f, 0.f }, desc->FaceCoordsFor(BlockFace::POS_Z));

				}
			}
		}

		// ^
		// Mesh
		// De Duplication
		// v

		chunk->Mesh = Drawing::RawMesh{ Drawing::VertexData::FromGeneric(Drawing::VoxelVertexDesc, vertices.begin(), vertices.end()), indices };
		auto deDupedMesh = MeshHelp::DeDuplicateVertices(Drawing::MeshView<Drawing::VoxelVertex>(chunk->Mesh));

		// ^
		// De Dup
		// Bvh
		// v

		if (deDupedMesh.VertexData.NumVertices())
		{
			size_t numVerts = deDupedMesh.VertexData.NumVertices();
			// Copy vertices to permanent buffer
			chunk->PhysicsPositions = std::make_unique<std::vector<floaty3>>();
			chunk->PhysicsPositions->reserve(numVerts);
			chunk->PhysicsIndices = std::make_unique<std::vector<unsigned int>>();
			chunk->PhysicsIndices->reserve(deDupedMesh.Indices.size());

			{
				Drawing::MeshView<Drawing::VoxelVertex> view{ deDupedMesh };
				for (size_t i = 0; i < numVerts; ++i)
				{
					chunk->PhysicsPositions->push_back(view[i].Position);
				}
				chunk->PhysicsIndices->insert(chunk->PhysicsIndices->begin(), deDupedMesh.Indices.begin(), deDupedMesh.Indices.end());
			}

			btIndexedMesh indexMesh;
			indexMesh.m_numTriangles = (int)chunk->PhysicsIndices->size() / 3;
			indexMesh.m_triangleIndexBase = reinterpret_cast<const unsigned char*>(chunk->PhysicsIndices->data());
			indexMesh.m_triangleIndexStride = sizeof(int) * 3;
			indexMesh.m_numVertices = (int)numVerts;
			indexMesh.m_vertexBase = reinterpret_cast<const unsigned char*>(chunk->PhysicsPositions->data());
			indexMesh.m_vertexStride = sizeof(floaty3);
			indexMesh.m_vertexType = PHY_FLOAT;

			chunk->PhysicsTriangles = std::make_shared<btTriangleIndexVertexArray>();

			chunk->PhysicsTriangles->addIndexedMesh(indexMesh, PHY_INTEGER);

			chunk->PhysicsShape = std::make_shared<btBvhTriangleMeshShape>(chunk->PhysicsTriangles.get(), true);
		}

		return chunk;
		//	chunk.PhysicsShape = std::make_shared<btBvhTriangleMeshShape>(chunk.PhysicsTriangles.get(), true);

		//	m_Body = std::make_shared<btCollisionObject>();

		//	m_Body->setUserPointer(&this->m_Holder);
		//	m_Body->setCollisionShape(m_Shape.get());

		//	Matrixy4x4 trans = Matrixy4x4::Translate(m_Origin);
		//	btTransform bTrans;
		//	bTrans.setFromOpenGLMatrix(trans.ma);
		//	m_Body->setWorldTransform(bTrans);

		//	Container->RequestPhysicsCall(m_Body, ENVIRONMENT, PLAYER | ENTITY_GENERAL);
		//}

		//// ^
		//// Bvh
		//// Submit drawcall
		//// v

		//auto mesh = Drawing::Mesh{ std::move(renderMesh), Drawing::MeshStorageType::DEDICATED_BUFFER };

		//if (m_Mesh)
		//	*m_Mesh = std::move(mesh);
		//else
		//	m_Mesh = std::make_shared<Drawing::Mesh>(std::move(mesh));
	}

	std::unique_ptr<LoadedChunk> GenerateChunkMesh(const ChunkData& data, ChunkCoord coord, std::function<SerialBlock(Voxel::BlockCoord coord)> blockDataFunc)
	{
		return GenerateChunkMeshT(data, coord, std::move(blockDataFunc));
	}
}