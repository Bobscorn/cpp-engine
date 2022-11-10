#include "VoxelChunk.h"

#include "Helpers/MeshHelper.h"
#include "Helpers/ProfileHelper.h"

#include "VoxelWorld.h"
#include "VoxelTypes.h"
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

	Voxel::VoxelChunk::VoxelChunk(G1::IGSpace* container, CommonResources* resources, VoxelWorld* world, floaty3 origin, ChunkCoord coord)
		: G1::IShape(container, CreateChunkName(coord))
		, FullResourceHolder(resources)
		, m_World(world)
		, m_Culler(CreateCuller(m_Origin))
		, m_Origin(origin)
		, m_Coord(coord)
		, m_Data()
		, m_UpdateBlocks()
	{
	}

	Voxel::VoxelChunk::VoxelChunk(G1::IGSpace* container, CommonResources* resources, VoxelWorld* world, floaty3 origin, RawChunkDataMap initial_dat, ChunkCoord coord)
		: G1::IShape(container, CreateChunkName(coord))
		, FullResourceHolder(resources)
		, m_World(world)
		, m_Culler(CreateCuller(m_Origin))
		, m_Origin(origin)
		, m_Data()
		, m_Material(Drawing::MaterialStore::Instance().GetMaterial("default-voxel-material"))
		, m_Coord(coord)
		, m_Mesh(std::make_shared<Drawing::Mesh>())
		, m_UpdateBlocks()
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

	VoxelChunk::VoxelChunk(G1::IGSpace* container, CommonResources* resources, VoxelWorld* world, floaty3 origin, std::unique_ptr<LoadedChunk> preloadedStuff)
		: G1::IShape(container, CreateChunkName(preloadedStuff->Coord))
		, FullResourceHolder(resources)
		, m_World(world)
		, m_Culler(CreateCuller(m_Origin))
		, m_Origin(origin)
		, m_Data()
		, m_Material(Drawing::MaterialStore::Instance().GetMaterial("default-voxel-material"))
		, m_Coord(preloadedStuff->Coord)
		, m_Mesh()
		, m_UpdateBlocks()
	{
		SetFrom(std::move(preloadedStuff));
		PROFILE_PUSH("Submitting DrawCall");
		
		m_DrawCall = resources->Ren3v2->SubmitDrawCall(Drawing::DrawCallv2{ m_Mesh, m_Material, std::make_shared<Matrixy4x4>(Matrixy4x4::Translate(m_Origin)), CreateChunkName(m_Coord), true});
		PROFILE_POP();
	}

	Voxel::VoxelChunk::~VoxelChunk()
	{
		for (auto& block : m_UpdateBlocks)
		{
			if (block.second)
				block.second->OnUnloaded();
		}
		if (m_Body)
		{
			Container->RequestPhysicsRemoval(m_Body.get());
		}
	}

	void Voxel::VoxelChunk::BeforeDraw()
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

	void Voxel::VoxelChunk::AfterDraw()
	{
		for (auto& updateBlockPair : m_UpdateBlocks)
		{
			if (updateBlockPair.second)
				updateBlockPair.second->AfterDraw();
		}
	}

	void Voxel::VoxelChunk::UpdateOrigin(floaty3 origin)
	{
		m_Origin = origin;
		m_Culler = CreateCuller(m_Origin);
		m_Dirty = true;
	}

	Voxel::ChunkyFrustumCuller* Voxel::VoxelChunk::GetCuller() const
	{
		return m_Culler.get();
	}

	void Voxel::VoxelChunk::set(uint32_t x, uint32_t y, uint32_t z, std::unique_ptr<Voxel::ICube> val)
	{
		ChunkBlockCoord key{ x, y, z };
		m_Data[x][y][z] = val->GetBlockData();
		auto& block = m_UpdateBlocks[key] = std::move(val);
		block->Attach(m_World, this, key);
		block->OnLoaded();
		block->OnPlaced();
		m_Dirty = true;
	}

	void VoxelChunk::set(ChunkBlockCoord coord, std::unique_ptr<Voxel::ICube> val)
	{
		set(coord.x, coord.y, coord.z, std::move(val));
	}

	void VoxelChunk::set(ChunkBlockCoord coord, const SerialBlock& block)
	{
		auto curIt = m_UpdateBlocks.find(coord);
		if (curIt != m_UpdateBlocks.end())
		{
			curIt->second->OnDestroyed();
			curIt->second->OnUnloaded();
			m_UpdateBlocks.erase(curIt);
		}
		m_Data[coord.x][coord.y][coord.z] = block;
		auto desc = VoxelStore::Instance().GetDescOrEmpty(block.ID);
		if (desc->WantsUpdate)
		{
			auto tmp = VoxelStore::Instance().CreateCube(m_World, this, coord, desc->Name);
			if (tmp)
			{
				auto& newBlock = m_UpdateBlocks[coord] = std::move(tmp);
				newBlock->OnLoaded();
				newBlock->OnPlaced();
			}
		}
		m_Dirty = true;
	}

	Voxel::ICube* Voxel::VoxelChunk::get(uint32_t x, uint32_t y, uint32_t z)
	{
		auto it = m_UpdateBlocks.find(ChunkBlockCoord{ x, y, z });
		if (it != m_UpdateBlocks.end())
			return it->second.get();
		return nullptr;
	}

	ICube* VoxelChunk::get(ChunkBlockCoord coord)
	{
		return get(coord.x, coord.y, coord.z);
	}

	const SerialBlock& VoxelChunk::get_data(ChunkBlockCoord coord) const
	{
		return m_Data[coord.x][coord.y][coord.z];
	}

	ChunkCoord VoxelChunk::GetCoord() const
	{
		return m_Coord;
	}

	std::unique_ptr<ICube> VoxelChunk::take(ChunkBlockCoord coord)
	{
		auto it = m_UpdateBlocks.find(coord);
		if (it != m_UpdateBlocks.end())
		{
			it->second->OnUnloaded();
			it->second->Detach(std::move(m_Data[coord.x][coord.y][coord.z]));
			m_Data[coord.x][coord.y][coord.z] = SerialBlock{};
			return std::move(it->second);
		}
		return nullptr;
	}

	void Voxel::VoxelChunk::SetTo(std::unique_ptr<ChunkData> data)
	{
		(void)data;
		// TODO: this method (or delete this method)
		std::unordered_map<std::string, std::shared_ptr<GeoThing>> geos;
		std::unordered_map<std::string, std::shared_ptr<Material>> mats;
		std::unordered_map<std::string, std::shared_ptr<btCollisionShape>> shapes;

		m_Dirty = true;
	}

	void VoxelChunk::SetFrom(std::unique_ptr<LoadedChunk> preLoadedChunk, bool constructCubes)
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
							auto pos = ChunkBlockCoord{ x,y,z };
							auto tmp = vox.CreateCube(m_World, this, pos, preLoadedChunk->ChunkDat[x][y][z]);
							if (tmp)
							{
								 m_UpdateBlocks[pos] = std::move(tmp);
							}
						}
					}
				}
			}
			for (auto& block : m_UpdateBlocks)
			{
				block.second->OnLoaded();
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

	void Voxel::VoxelChunk::RecomputeMesh()
	{
		if (!m_World)
		{
			DERROR("Can not recompute mesh without a voxel world!");
			return;
		}

		auto& world = *m_World;

		SetFrom(GenerateChunkMesh(m_Data, m_Coord, [&world](BlockCoord coord) { return world.GetCubeDataAt(coord); }), false);
	}

	std::string VoxelChunk::CreateChunkName(ChunkCoord coord)
	{
		std::string name{ 0, (char)0, std::allocator<char>() };
		name.reserve(35);
		name.append("Chunk (").append(std::to_string(coord.X)).append(", ").append(std::to_string(coord.Y)).append(", ").append(std::to_string(coord.Z)).append(")");
		return name;
	}

	std::unique_ptr<ChunkyFrustumCuller> VoxelChunk::CreateCuller(floaty3 origin)
	{
		return std::make_unique<ChunkyFrustumCuller>(origin, floaty3{ Voxel::Chunk_Size, Voxel::Chunk_Height, Voxel::Chunk_Size });
	}

	std::unique_ptr<LoadedChunk> GenerateChunkMeshT(const ChunkData& data, ChunkCoord coord, std::function<SerialBlock(BlockCoord coord)> blockDataFunc)
	{
		std::unique_ptr<Voxel::LoadedChunk> chunk = std::make_unique<Voxel::LoadedChunk>();

		chunk->Coord = coord;

		std::vector<Voxel::VoxelVertex> vertices;
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
				return vox.GetDescOrEmpty(blockDataFunc(convertRelToCoord(chunkRelativeX, chunkRelativeY, chunkRelativeZ)).ID)->FaceOpaqueness[(int)face] == FaceClosedNess::CLOSED_FACE;
			return vox.GetDescOrEmpty(data[chunkRelativeX][chunkRelativeY][chunkRelativeZ].ID)->FaceOpaqueness[(int)face] == FaceClosedNess::CLOSED_FACE;
		};

		auto origin = coord;

		auto AddVerticesFunc = [&vertices, &indices, origin](const VoxelBlock* block, int chunkRelativeX, int chunkRelativeY, int chunkRelativeZ, BlockFace face, const CubeData& data)
		{
			// Generate vertices
			constexpr auto blockOffset = floaty3{ 0.5f * BlockSize, 0.5f * BlockSize, 0.5f * BlockSize }; // Offset necessary to make any origin cubes actually on the origin
			auto base = floaty3{ (float)chunkRelativeX * BlockSize, (float)chunkRelativeY * BlockSize, (float)chunkRelativeZ * BlockSize } + blockOffset;

			auto& verts = block->Mesh.FaceVertices[(size_t)face];
			auto& block_indices = block->Mesh.FaceIndices[(size_t)face];
			
			auto rot = data.Rotation;

			auto index_base = (unsigned int)vertices.size();
			for (Voxel::VoxelVertex vert : verts)
			{
				vert.Position = rot.rotate(vert.Position);
				vert.Normal = rot.rotate(vert.Normal);
				vert.Tangent = rot.rotate(vert.Tangent);
				vert.Binormal = rot.rotate(vert.Binormal);
				vert.Position += base;
				vertices.push_back(vert);
			}

			for (auto& index : block_indices)
			{
				indices.push_back(index + index_base);
			}
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

					for (auto& face : BlockFacesArray)
					{
						if (desc->FaceOpaqueness[(size_t)face] == FaceClosedNess::OPEN_FACE)
						{
							AddVerticesFunc(desc, x, y, z, face, blockDat.Data);
							continue;
						}

						auto rot = blockDat.Data.Rotation;

						auto dir = BlockFaceHelper::GetDirectionI(face);
						auto rotatedDir = rot.rotate(dir);
						auto rotatedFace = Voxel::RotateFace(face, rot);
						Vector::inty3 neighbourPos = Vector::inty3{ x, y, z } + rotatedDir;					
						if (!cubeAtHasFace(neighbourPos.x, neighbourPos.y, neighbourPos.z, rotatedFace))
							AddVerticesFunc(desc, x, y, z, face, blockDat.Data);
					}
				}
			}
		}

		// ^
		// Mesh
		// De Duplication
		// v

		chunk->Mesh = Drawing::RawMesh{ Drawing::VertexData::FromGeneric(Voxel::VoxelVertexDesc, vertices.begin(), vertices.end()), indices };
		auto deDupedMesh = MeshHelp::DeDuplicateVertices(Drawing::MeshView<Voxel::VoxelVertex>(chunk->Mesh));

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
				Drawing::MeshView<Voxel::VoxelVertex> view{ deDupedMesh };
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