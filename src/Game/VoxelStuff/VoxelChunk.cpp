#include "VoxelChunk.h"

#include "Helpers/MeshHelper.h"

#include "VoxelWorld.h"
#include "Drawing/VoxelStore.h"

#include "Structure/BulletBitmasks.h"

#include <algorithm>

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

	Voxel::RawChunkData ConvertMapToData(const Voxel::RawChunkDataMap& map)
	{
		Voxel::RawChunkData data{};

		for (auto& b : map)
		{
			auto& key = b.first;
			data[key.X][key.Y][key.Z] = b.second;
		}

		return data;
	}

	Voxel::ChunkyBoi::ChunkyBoi(G1::IGSpace* container, CommonResources* resources, VoxelWorld* world, floaty3 origin, ChunkCoord coord)
		: G1::IShape(container)
		, FullResourceHolder(resources)
		, m_World(world)
		, m_Culler(CreateCuller(m_Origin))
		, m_Origin(origin)
		, m_Coord(coord)
	{
	}

	Voxel::ChunkyBoi::ChunkyBoi(G1::IGSpace* container, CommonResources* resources, VoxelWorld* world, floaty3 origin, RawChunkData initial_dat, ChunkCoord coord)
		: G1::IShape(container)
		, FullResourceHolder(resources)
		, m_World(world)
		, m_Culler(CreateCuller(m_Origin))
		, m_Origin(origin)
		, m_Data()
		, m_Coord(coord)
	{
		PROFILE_PUSH("Creating Blocks");
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
		PROFILE_POP();
	}

	Voxel::ChunkyBoi::ChunkyBoi(G1::IGSpace* container, CommonResources* resources, VoxelWorld* world, floaty3 origin, RawChunkDataMap initial_dat, ChunkCoord coord)
		: G1::IShape(container)
		, FullResourceHolder(resources)
		, m_World(world)
		, m_Culler(CreateCuller(m_Origin))
		, m_Origin(origin)
		, m_Data()
		, m_Material(Drawing::MaterialStore::Instance().GetMaterial("example-voxel-material"))
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
		SetFrom(GenerateChunkMesh(ConvertMapToData(initial_dat), coord, [world = this->m_World](BlockCoord coord) { return world->GetCubeIdAt(coord); }));
		PROFILE_PUSH("Submitting DrawCall");
		auto name = std::string("Chunk (") + std::to_string(coord.X) + ", " + std::to_string(coord.Y) + ", " + std::to_string(coord.Z) + ")";
		m_DrawCall = resources->Ren3v2->SubmitDrawCall(Drawing::DrawCallv2{ m_Mesh, m_Material, std::make_shared<Matrixy4x4>(Matrixy4x4::Translate(m_Origin)), name, true });
		PROFILE_POP();
	}

	ChunkyBoi::ChunkyBoi(G1::IGSpace* container, CommonResources* resources, VoxelWorld* world, floaty3 origin, LoadedChunk preloadedStuff)
		: G1::IShape(container, "Chunk")
		, FullResourceHolder(resources)
		, m_World(world)
		, m_Culler(CreateCuller(m_Origin))
		, m_Origin(origin)
		, m_Data()
		, m_Material(Drawing::MaterialStore::Instance().GetMaterial("example-voxel-material"))
		, m_Coord(preloadedStuff.Coord)
		, m_Mesh()
	{
		SetFrom(std::move(preloadedStuff));
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

		for (auto& updateBlock : m_UpdateBlocks)
		{
			auto& block = m_Data[updateBlock.X][updateBlock.Y][updateBlock.Z];
			if (block)
				block->BeforeDraw();
		}

		if (m_Dirty)
		{
			RecomputeMesh();
			m_Dirty = false;
		}
	}

	void Voxel::ChunkyBoi::AfterDraw()
	{
		for (auto& updateBlock : m_UpdateBlocks)
		{
			auto& block = m_Data[updateBlock.X][updateBlock.Y][updateBlock.Z];
			if (block)
				block->AfterDraw();
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
					auto& block = m_Data[x][y][z];
					if (block)
						block->UpdatePosition(PositionFromCoord(m_Origin, x, y, z), x, y, z, m_Culler.get());
				}
			}
		}
		m_Dirty = true;
	}

	Voxel::ChunkyFrustumCuller* Voxel::ChunkyBoi::GetCuller() const
	{
		return m_Culler.get();
	}

	void Voxel::ChunkyBoi::set(size_t x, size_t y, size_t z, std::unique_ptr<Voxel::ICube> val)
	{
		ChunkBlockKey key{ x, y, z };
		auto it = std::find_if(m_UpdateBlocks.begin(), m_UpdateBlocks.end(), [&key](ChunkBlockKey a) { return a == key; });
		if (it != m_UpdateBlocks.end())
		{
			std::swap(m_UpdateBlocks[std::distance(m_UpdateBlocks.begin(), it)], m_UpdateBlocks.back());
			m_UpdateBlocks.pop_back();
		}
		m_Data[x][y][z] = std::move(val);
		if (m_Data[x][y][z] && m_Data[x][y][z]->WantsUpdate())
			m_UpdateBlocks.emplace_back(key);
		m_Dirty = true;
	}

	void Voxel::ChunkyBoi::create(size_t x, size_t y, size_t z)
	{
		set(x, y, z, std::make_unique<Voxel::VoxelCube>(this->Container, this->mResources, this->m_World, PositionFromCoord(m_Origin, x, y, z), x, y, z));
	}

	Voxel::ICube* Voxel::ChunkyBoi::get(size_t x, size_t y, size_t z)
	{
		if (x >= Chunk_Size || y >= Chunk_Height || z >= Chunk_Size)
			return nullptr;
		return m_Data[x][y][z].get();
	}

	void Voxel::ChunkyBoi::SetTo(RawChunkData data)
	{
		// TODO: this method (or delete this method)
		std::unordered_map<std::string, std::shared_ptr<GeoThing>> geos;
		std::unordered_map<std::string, std::shared_ptr<Material>> mats;
		std::unordered_map<std::string, std::shared_ptr<btCollisionShape>> shapes;

		m_Dirty = true;
	}

	void ChunkyBoi::SetFrom(LoadedChunk preLoadedChunk, bool constructCubes)
	{
		if (constructCubes)
		{
			m_UpdateBlocks.clear();
			for (int x = 0; x < Chunk_Size; ++x)
			{
				for (int y = 0; y < Chunk_Height; ++y)
				{
					for (int z = 0; z < Chunk_Size; ++z)
					{
						auto& block = m_Data[x][y][z];
						block = VoxelStore::Instance().CreateCube(preLoadedChunk.ChunkDat[x][y][z].ID);
						if (block)
						{
							if (block->WantsUpdate())
								m_UpdateBlocks.emplace_back(ChunkBlockKey{ x, y, z });
							block->UpdatePosition(PositionFromCoord(m_Origin, x, y, z), x, y, z, nullptr);
						}
					}
				}
			}
		}

		auto m = Drawing::Mesh{ std::move(preLoadedChunk.Mesh), Drawing::MeshStorageType::DEDICATED_BUFFER };

		if (m_Mesh)
			*m_Mesh = std::move(m);
		else
			m_Mesh = std::make_shared<Drawing::Mesh>(std::move(m));

		if (m_Body)
			Container->RequestPhysicsRemoval(m_Body.get());
		m_Body = nullptr;

		if (preLoadedChunk.PhysicsIndices)
			m_PhysIndices.swap(*preLoadedChunk.PhysicsIndices);
		else
			m_PhysIndices.clear();
		if (preLoadedChunk.PhysicsPositions)
			m_PhysVertices.swap(*preLoadedChunk.PhysicsPositions);
		else
			m_PhysVertices.clear();

		m_MeshData = std::move(preLoadedChunk.PhysicsTriangles);
		m_Shape = std::move(preLoadedChunk.PhysicsShape);

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
	}

	void Voxel::ChunkyBoi::RecomputeMesh()
	{
		if (!m_World)
			return;

		auto& world = *m_World;

		SetFrom(GenerateChunkMesh(m_Data, m_Coord, [&world](BlockCoord coord) { return world.GetCubeIdAt(coord); }), false);
	}

	std::unique_ptr<ChunkyFrustumCuller> ChunkyBoi::CreateCuller(floaty3 origin)
	{
		return std::make_unique<ChunkyFrustumCuller>(origin, floaty3{ Voxel::Chunk_Size, Voxel::Chunk_Height, Voxel::Chunk_Size });
	}

	template<class T>
	LoadedChunk GenerateChunkMeshT(const T& data, ChunkCoord coord, std::function<size_t(BlockCoord coord)> blockIdFunc)
	{
		static_assert(std::is_same_v<T, ChunkData> || std::is_same_v<T, RawChunkData>, "Hidden template function not being used with known classes");
		constexpr bool isRaw = std::is_same_v<T, RawChunkData>;
		Voxel::LoadedChunk chunk;

		chunk.Coord = coord;
		if constexpr (std::is_same_v<T, Voxel::RawChunkData>)
			chunk.ChunkDat = data;

		std::vector<Drawing::VoxelVertex> vertices;
		std::vector<GLuint> indices;

		// Actual generate mesh
		// v

		// Simplest method: just add 3 vertices and 3 indices for each exposed face of a block

		auto convertRelToCoord = [coord](int x, int y, int z)
		{
			BlockCoord co;
			co.Chunk = coord;
			co.X = x;
			co.Y = y;
			co.Z = z;
			if (x < 0)
			{
				--co.Chunk.X;
				co.X = Chunk_Size - 1;
			}
			if (y < 0)
			{
				--co.Chunk.Y;
				co.Y = Chunk_Height - 1;
			}
			if (z < 0)
			{
				--co.Chunk.Z;
				co.Z = Chunk_Size - 1;
			}
			if (x >= (int)Chunk_Size)
			{
				++co.Chunk.X;
				co.X = 0;
			}
			if (y >= (int)Chunk_Height)
			{
				++co.Chunk.Y;
				co.Y = 0;
			}
			if (z >= (int)Chunk_Size)
			{
				++co.Chunk.Z;
				co.Z = 0;
			}
			return co;
		};


		auto IsCubeAtFunc = [&blockIdFunc, &data, &convertRelToCoord, coord](int chunkRelativeX, int chunkRelativeY, int chunkRelativeZ)
		{
			constexpr bool isRaw = std::is_same_v<T, RawChunkData>;
			if (chunkRelativeX < 0 || chunkRelativeY < 0 || chunkRelativeZ < 0 ||
				chunkRelativeX >= Chunk_Size || chunkRelativeY >= Chunk_Height || chunkRelativeZ >= Chunk_Size)
				return VoxelStore::Instance().GetDescOrEmpty(blockIdFunc(convertRelToCoord(chunkRelativeX, chunkRelativeY, chunkRelativeZ))).isOpaque;
			if constexpr (isRaw)
				return VoxelStore::Instance().GetDescOrEmpty(data[chunkRelativeX][chunkRelativeY][chunkRelativeZ].ID).isOpaque;
			else
				return VoxelStore::Instance().GetDescOrEmpty(VoxelStore::Instance().GetIDFor(data[chunkRelativeX][chunkRelativeY][chunkRelativeZ] ? data[chunkRelativeX][chunkRelativeY][chunkRelativeZ]->GetBlockName() : "")).isOpaque;
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

			auto indexOffset = vertices.size();

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
					size_t blockID;
					if constexpr (isRaw)
						blockID = data[x][y][z].ID;
					else
						blockID = VoxelStore::Instance().GetIDFor(data[x][y][z] ? data[x][y][z]->GetBlockName() : "");

					if (blockID == 0)
						continue;

					VoxelBlock desc = VoxelStore::Instance().GetDescOrEmpty(blockID);

					// go through neighbours check if there is a block there
					// if there isn't add triangles to mesh

					// -X
					if (!IsCubeAtFunc(x - 1, y, z))
						AddVerticesFunc(x, y, z, floaty3{ -1.f, 0.f, 0.f }, floaty3{ 0.f, 0.f, -1.f }, desc.FaceCoordsFor(BlockFace::NEG_X));
					// +X
					if (!IsCubeAtFunc(x + 1, y, z))
						AddVerticesFunc(x, y, z, floaty3{ +1.f, 0.f, 0.f }, floaty3{ 0.f, 0.f, +1.f }, desc.FaceCoordsFor(BlockFace::POS_X));
					// -Y
					if (!IsCubeAtFunc(x, y - 1, z))
						AddVerticesFunc(x, y, z, floaty3{ 0.f, -1.f, 0.f }, floaty3{ +1.f, 0.f, 0.f }, desc.FaceCoordsFor(BlockFace::NEG_Y));
					// +Y
					if (!IsCubeAtFunc(x, y + 1, z))
						AddVerticesFunc(x, y, z, floaty3{ 0.f, +1.f, 0.f }, floaty3{ -1.f, 0.f, 0.f }, desc.FaceCoordsFor(BlockFace::POS_Y));
					// -Z
					if (!IsCubeAtFunc(x, y, z - 1))
						AddVerticesFunc(x, y, z, floaty3{ 0.f, 0.f, -1.f }, floaty3{ +1.f, 0.f, 0.f }, desc.FaceCoordsFor(BlockFace::NEG_Z));
					// +Z
					if (!IsCubeAtFunc(x, y, z + 1))
						AddVerticesFunc(x, y, z, floaty3{ 0.f, 0.f, +1.f }, floaty3{ -1.f, 0.f, 0.f }, desc.FaceCoordsFor(BlockFace::POS_Z));

				}
			}
		}

		// ^
		// Mesh
		// De Duplication
		// v

		auto curCount = vertices.size();
		chunk.Mesh = Drawing::RawMesh{ Drawing::VertexData::FromGeneric(Drawing::VoxelVertexDesc, vertices.begin(), vertices.end()), indices };
		auto deDupedMesh = MeshHelp::DeDuplicateVertices(Drawing::MeshView<Drawing::VoxelVertex>(chunk.Mesh));
		auto newCount = deDupedMesh.VertexData.NumVertices();

		// ^
		// De Dup
		// Bvh
		// v

		if (deDupedMesh.VertexData.NumVertices())
		{
			int numVerts = deDupedMesh.VertexData.NumVertices();
			// Copy vertices to permanent buffer
			chunk.PhysicsPositions = std::make_unique<std::vector<floaty3>>();
			chunk.PhysicsPositions->reserve(numVerts);
			chunk.PhysicsIndices = std::make_unique<std::vector<unsigned int>>();
			chunk.PhysicsIndices->reserve(deDupedMesh.Indices.size());

			{
				Drawing::MeshView<Drawing::VoxelVertex> view{ deDupedMesh };
				for (int i = 0; i < numVerts; ++i)
				{
					chunk.PhysicsPositions->push_back(view[i].Position);
				}
				chunk.PhysicsIndices->insert(chunk.PhysicsIndices->begin(), deDupedMesh.Indices.begin(), deDupedMesh.Indices.end());
			}

			btIndexedMesh indexMesh;
			indexMesh.m_numTriangles = chunk.PhysicsIndices->size() / 3;
			indexMesh.m_triangleIndexBase = reinterpret_cast<const unsigned char*>(chunk.PhysicsIndices->data());
			indexMesh.m_triangleIndexStride = sizeof(int) * 3;
			indexMesh.m_numVertices = numVerts;
			indexMesh.m_vertexBase = reinterpret_cast<const unsigned char*>(chunk.PhysicsPositions->data());
			indexMesh.m_vertexStride = sizeof(floaty3);
			indexMesh.m_vertexType = PHY_FLOAT;

			chunk.PhysicsTriangles = std::make_shared<btTriangleIndexVertexArray>();

			chunk.PhysicsTriangles->addIndexedMesh(indexMesh, PHY_INTEGER);

			chunk.PhysicsShape = std::make_shared<btBvhTriangleMeshShape>(chunk.PhysicsTriangles.get(), true);
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

	LoadedChunk GenerateChunkMesh(const RawChunkData& data, ChunkCoord coord, std::function<size_t(Voxel::BlockCoord coord)> blockIdFunc)
	{
		return GenerateChunkMeshT<RawChunkData>(data, coord, std::move(blockIdFunc));
	}

	LoadedChunk GenerateChunkMesh(const ChunkData& data, ChunkCoord coord, std::function<size_t(Voxel::BlockCoord coord)> blockIdFunc)
	{
		return GenerateChunkMeshT<ChunkData>(data, coord, std::move(blockIdFunc));
	}
}