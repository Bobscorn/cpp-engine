#include "VoxelChunk.h"

#include "VoxelWorld.h"

#include "Structure/BulletBitmasks.h"

constexpr floaty3 OriginFromCoord(Voxel::ChunkCoord coord)
{
	return { ((float)Voxel::Chunk_Size * -0.5f) - 0.5f + (float)((int64_t)Voxel::Chunk_Size * coord.X), ((float)Voxel::Chunk_Size * -0.5f) - 0.5f + (float)((int64_t)Voxel::Chunk_Size * coord.Y), ((float)Voxel::Chunk_Size * -0.5f) - 0.5f + (float)((int64_t)Voxel::Chunk_Size * coord.Z) };
}

constexpr floaty3 PositionFromCoord(floaty3 origin, size_t x, size_t y, size_t z)
{
	return { origin.x + ((float)x + 0.5f) * Voxel::BlockSize, origin.y + ((float)y + 0.5f) * Voxel::BlockSize, origin.z + ((float)z + 0.5f) * Voxel::BlockSize };
}

Voxel::ChunkyBoi::ChunkyBoi(G1::IGSpace *container, CommonResources *resources, VoxelWorld *world, floaty3 origin, ChunkCoord coord) 
	: G1::IShape(container)
	, FullResourceHolder(resources)
	, m_World(world)
	, m_Culler(CreateCuller(m_Origin))
	, m_Origin(origin)
	, m_Coord(coord)
{
}

Voxel::ChunkyBoi::ChunkyBoi(G1::IGSpace *container, CommonResources *resources, VoxelWorld *world, floaty3 origin, RawChunkData initial_dat, ChunkCoord coord) 
	: G1::IShape(container)
	, FullResourceHolder(resources)
	, m_World(world)
	, m_Culler(CreateCuller(m_Origin))
	, m_Origin(origin)
	, m_Data()
	, m_Coord(coord)
{
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
}

Voxel::ChunkyBoi::ChunkyBoi(G1::IGSpace *container, CommonResources *resources, VoxelWorld *world, floaty3 origin, RawChunkDataMap initial_dat, ChunkCoord coord) 
	: G1::IShape(container)
	, FullResourceHolder(resources)
	, m_World(world)
	, m_Culler(CreateCuller(m_Origin))
	, m_Origin(origin)
	, m_Data()
	, m_Material(Drawing::MaterialStore::Instance().GetMaterial("Default3D"))
	, m_Coord(coord)
{
	for (auto &pair : initial_dat)
	{
		if (pair.second.ID)
		{
			create(pair.first.X, pair.first.Y, pair.first.Z);
		}
	}
	RecomputeMesh();
	if (m_Mesh->GetMesh() && m_Mesh->GetMesh()->VertexData.NumVertices())
	{
		auto name = std::string("Chunk (") + std::to_string(coord.X) + ", " + std::to_string(coord.Y) + ", " + std::to_string(coord.Z) + ")";
		m_DrawCall = resources->Ren3v2->SubmitDrawCall(Drawing::DrawCallv2{ m_Mesh, m_Material, std::make_shared<Matrixy4x4>(Matrixy4x4::Translate(m_Origin)), name });
	}
}

Voxel::ChunkyBoi::~ChunkyBoi()
{
}

void Voxel::ChunkyBoi::BeforeDraw()
{
	if (m_Culler)
		m_Culler->Flush();

	for (auto &plane : m_Data)
	{
		for (auto &line : plane)
		{
			for (auto &block : line)
			{
				if (block)
					block->BeforeDraw();
			}
		}
	}
}

void Voxel::ChunkyBoi::AfterDraw()
{


	for (auto &plane : m_Data)
	{
		for (auto &line : plane)
		{
			for (auto &block : line)
			{
				if (block)
					block->AfterDraw();
			}
		}
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
				auto &block = m_Data[x][y][z];
				if (block)
					block->UpdatePosition(PositionFromCoord(m_Origin, x, y, z), x, y, z, m_Culler.get());
			}
		}
	}
	RecomputeMesh();
}

Voxel::ChunkyFrustumCuller *Voxel::ChunkyBoi::GetCuller() const
{
	return m_Culler.get();
}

void Voxel::ChunkyBoi::set(size_t x, size_t y, size_t z, std::unique_ptr<Voxel::ICube> val)
{
	m_Data[x][y][z] = std::move(val);
	RecomputeMesh();
}

void Voxel::ChunkyBoi::create(size_t x, size_t y, size_t z)
{
	m_Data[x][y][z] = std::make_unique<Voxel::VoxelCube>(this->Container, this->mResources, this->m_World, PositionFromCoord(m_Origin, x, y, z), x, y, z);
	RecomputeMesh();
}

Voxel::ICube* Voxel::ChunkyBoi::get(size_t x, size_t y, size_t z)
{
	if (x >= Chunk_Size || y >= Chunk_Height || z >= Chunk_Size)
		return nullptr;
	return m_Data[x][y][z].get();
}

void Voxel::ChunkyBoi::SetTo(RawChunkData data)
{
	std::unordered_map<std::string, std::shared_ptr<GeoThing>> geos;
	std::unordered_map<std::string, std::shared_ptr<Material>> mats;
	std::unordered_map<std::string, std::shared_ptr<btCollisionShape>> shapes;

	// Possibly implement dirty tag?
	RecomputeMesh();
}

void Voxel::ChunkyBoi::RecomputeMesh()
{
	if (!m_World)
		return;

	auto& world = *m_World;

	Drawing::RawMesh uwu;

	std::vector<Drawing::Full3DVertex> vertices;
	std::vector<GLuint> indices;

	// Actual generate mesh
	// v

	// Simplest method: just add 3 vertices and 3 indices for each exposed face of a block
	auto coord = m_Coord;

	auto convertRelToCoord = [coord](int x, int y, int z)
	{
		BlockCoord co;
		co.Chunk = coord;
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
		if (x > Chunk_Size)
		{
			++co.Chunk.X;
			co.X = 0;
		}
		if (y > Chunk_Height)
		{
			++co.Chunk.Y;
			co.Y = 0;
		}
		if (z > Chunk_Size)
		{
			++co.Chunk.Z;
			co.Z = 0;
		}
		return co;
	};

	auto& dat = m_Data;

	auto IsCubeAtFunc = [&world, &dat, &convertRelToCoord, coord](int chunkRelativeX, int chunkRelativeY, int chunkRelativeZ)
	{
		if (chunkRelativeX < 0 || chunkRelativeY < 0 || chunkRelativeZ < 0 ||
			chunkRelativeX >= Chunk_Size || chunkRelativeY >= Chunk_Height || chunkRelativeZ >= Chunk_Size)
			return world.IsCubeAt(convertRelToCoord(chunkRelativeX, chunkRelativeY, chunkRelativeZ));
		return dat[chunkRelativeX][chunkRelativeY][chunkRelativeZ].get() != nullptr;
	};

	auto origin = m_Origin;

	auto AddVerticesFunc = [&vertices, &indices, origin](int chunkRelativeX, int chunkRelativeY, int chunkRelativeZ, floaty3 direction, floaty3 tangent)
	{
		auto binorm = direction.cross(tangent);

		// Generate vertices
		constexpr auto blockOffset = floaty3{ 0.5f * BlockSize, 0.5f * BlockSize, 0.5f * BlockSize }; // Offset necessary to make any origin cubes actually on the origin
		auto base = floaty3{ (float)chunkRelativeX * BlockSize, (float)chunkRelativeY * BlockSize, (float)chunkRelativeZ * BlockSize } + blockOffset;

		// Generative base vertex for copying
		Drawing::Full3DVertex vert1;
		vert1.Position = base + direction * 0.5f * BlockSize + tangent * 0.5f * BlockSize + binorm * 0.5f * BlockSize;
		vert1.Normal = direction;
		vert1.Tangent = tangent;
		vert1.Binormal = binorm;
		vert1.TexCoord = { 1.f, 1.f };

		Drawing::Full3DVertex vert2 = vert1;
		vert2.Position = base + direction * 0.5f * BlockSize + tangent * -0.5f * BlockSize + binorm * 0.5f * BlockSize;
		vert2.TexCoord = { 0.f, 1.f };

		Drawing::Full3DVertex vert3 = vert1;
		vert3.Position = base + direction * 0.5f * BlockSize + tangent * -0.5f * BlockSize + binorm * -0.5f * BlockSize;
		vert3.TexCoord = { 0.f, 0.f };

		Drawing::Full3DVertex vert4 = vert1;
		vert4.Position = base + direction * 0.5f * BlockSize + tangent * 0.5f * BlockSize + binorm * -0.5f * BlockSize;
		vert4.TexCoord = { 1.f, 0.f };

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
		for (int y = 0; y < Chunk_Size; ++y)
		{
			for (int z = 0; z < Chunk_Size; ++z)
			{
				auto& block = m_Data[x][y][z];

				if (!block.get())
					continue;

				// go through neighbours check if there is a block there
				// if there isn't add triangles to mesh

				// -X
				if (!IsCubeAtFunc(x - 1, y, z))
					AddVerticesFunc(x, y, z, floaty3{ -1.f, 0.f, 0.f }, floaty3{ 0.f, 0.f, +1.f });
				// +X
				if (!IsCubeAtFunc(x + 1, y, z))
					AddVerticesFunc(x, y, z, floaty3{ +1.f, 0.f, 0.f }, floaty3{ 0.f, 0.f, -1.f });
				// -Y
				if (!IsCubeAtFunc(x, y - 1, z))
					AddVerticesFunc(x, y, z, floaty3{ 0.f, -1.f, 0.f }, floaty3{ -1.f, 0.f, 0.f });
				// +Y
				if (!IsCubeAtFunc(x, y + 1, z))
					AddVerticesFunc(x, y, z, floaty3{ 0.f, +1.f, 0.f }, floaty3{ +1.f, 0.f, 0.f });
				// -Z
				if (!IsCubeAtFunc(x, y, z - 1))
					AddVerticesFunc(x, y, z, floaty3{ 0.f, 0.f, -1.f }, floaty3{ 0.f, -1.f, 0.f });
				// +Z
				if (!IsCubeAtFunc(x, y, z + 1))
					AddVerticesFunc(x, y, z, floaty3{ 0.f, 0.f, +1.f }, floaty3{ 0.f, +1.f, 0.f });

			}
		}
	}
	
	// ^
	// Mesh

	auto mesh = Drawing::Mesh{ Drawing::RawMesh{ Drawing::VertexData::FromFull3DVertices(vertices), indices }, Drawing::MeshStorageType::DEDICATED_BUFFER };

	if (m_Mesh)
		*m_Mesh = std::move(mesh);
	else
		m_Mesh = std::make_shared<Drawing::Mesh>(std::move(mesh));
}

std::unique_ptr<Voxel::ChunkyFrustumCuller> Voxel::ChunkyBoi::CreateCuller(floaty3 origin)
{
	return std::make_unique<Voxel::ChunkyFrustumCuller>(origin, floaty3{ Voxel::Chunk_Size, Voxel::Chunk_Height, Voxel::Chunk_Size });
}
