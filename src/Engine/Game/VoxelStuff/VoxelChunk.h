#pragma once

#include "VoxelValues.h"

#include "VoxelTypes.h"
#include "VoxelCube.h"
#include "VoxelChunkCuller.h"

#include "Drawing/Mesh.h"
#include "Drawing/Geometry.h"
#include "Drawing/Material.h"
#include "Drawing/DrawCallReference.h"

#include <BulletCollision/CollisionShapes/btBoxShape.h>

#include <vector>
#include <memory>
#include <functional>
#include <unordered_map>

namespace std
{
	template<>
	struct hash<Voxel::ChunkCoord>
	{
		inline size_t operator()(const Voxel::ChunkCoord &in) const
		{
			size_t out = 0;
			out |= (((size_t)in.X + (size_t)LLONG_MAX) >> 42) >> 42;
			out |= (((size_t)in.Y + (size_t)LLONG_MAX) >> 42) << 21;
			out |= (((size_t)in.Z + (size_t)LLONG_MAX) >> 42) << 42;
			return out;
		}
	};

	template<>
	struct hash<Voxel::ChunkBlockCoord>
	{
		inline size_t operator()(const Voxel::ChunkBlockCoord& in) const
		{
			size_t out = 0;
			out |= (((size_t)in.x + (size_t)LLONG_MAX) >> 42) >> 42;
			out |= (((size_t)in.y + (size_t)LLONG_MAX) >> 42) << 21;
			out |= (((size_t)in.z + (size_t)LLONG_MAX) >> 42) << 42;
			return out;
		}
	};
}

namespace Voxel
{
	class VoxelWorld;

	typedef std::unordered_map<ChunkBlockCoord, SerialBlock> RawChunkDataMap;

	typedef std::array<std::array<std::array<SerialBlock, Chunk_Size>, Chunk_Height>, Chunk_Size> ChunkData;
	std::unique_ptr<ChunkData> ConvertMapToData(const RawChunkDataMap& m);

	struct LoadedChunk
	{
		ChunkCoord Coord;
		ChunkData ChunkDat;
		std::unique_ptr<std::vector<floaty3>> PhysicsPositions;
		std::unique_ptr<std::vector<unsigned int>> PhysicsIndices;
		std::shared_ptr<btTriangleIndexVertexArray> PhysicsTriangles;
		std::shared_ptr<btBvhTriangleMeshShape> PhysicsShape;
		Drawing::RawMesh Mesh;
	};

	class VoxelChunk : public G1::IShape, public BulletHelp::INothingInterface
	{
	public:

		VoxelChunk(G1::IGSpace* container, CommonResources* resources, VoxelWorld* world, floaty3 origin, ChunkCoord coord);
		VoxelChunk(G1::IGSpace* container, CommonResources* resources, VoxelWorld* world, floaty3 origin, RawChunkDataMap initial_dat, ChunkCoord coord);
		VoxelChunk(G1::IGSpace* container, CommonResources* resources, VoxelWorld* world, floaty3 origin, std::unique_ptr<LoadedChunk> preLoadedData);
		~VoxelChunk();

		void BeforeDraw() override;
		void AfterDraw() override;

		void UpdateOrigin(floaty3 new_origin);

		ChunkyFrustumCuller *GetCuller() const;

		//inline operator std::array<std::array<std::array<std::unique_ptr<Voxel::ICube>, Chunk_Size>, Chunk_Height>, Chunk_Size> &const() { return m_Data; }
		
		// Will set the targetted block and add it to m_UpdateBlocks
		void set(uint32_t x, uint32_t y, uint32_t z, std::unique_ptr<Voxel::ICube> val);
		void set(ChunkBlockCoord coord, std::unique_ptr<Voxel::ICube> val);

		void set(ChunkBlockCoord coord, const SerialBlock& block);

		ICube* get(uint32_t x, uint32_t y, uint32_t z);
		ICube* get(ChunkBlockCoord coord);
		const SerialBlock& get_data(ChunkBlockCoord coord) const;
		ChunkCoord GetCoord() const;

		// Use with caution, will remove the block from this chunk
		std::unique_ptr<ICube> take(ChunkBlockCoord coord);

		void SetTo(std::unique_ptr<ChunkData> data);

		void SetFrom(std::unique_ptr<LoadedChunk> preLoadedChunk, bool constructCubes = true);

		void RecomputeMesh();

	protected:
		
		std::string CreateChunkName(ChunkCoord coord);

		std::unique_ptr<ChunkyFrustumCuller> CreateCuller(floaty3 origin);

		ChunkData m_Data;
		std::unordered_map<ChunkBlockCoord, std::unique_ptr<ICube>> m_UpdateBlocks;
		floaty3 m_Origin;
		VoxelWorld *m_World = nullptr;
		ChunkCoord m_Coord;

		std::unique_ptr<ChunkyFrustumCuller> m_Culler;

		std::shared_ptr<Drawing::Material> m_Material;

		std::vector<floaty3> m_PhysVertices;
		std::vector<GLuint> m_PhysIndices;
		std::shared_ptr<btTriangleIndexVertexArray> m_MeshData;
		std::shared_ptr<btBvhTriangleMeshShape> m_Shape;
		std::shared_ptr<btCollisionObject> m_Body;

		// v2 Rendering stuff
		std::shared_ptr<Drawing::Mesh> m_Mesh;
		std::shared_ptr<Drawing::Material> m_Matv2;
		std::shared_ptr<Matrixy4x4> m_Matrix;

		Drawing::DrawCallReference m_DrawCall;

		bool m_Dirty = false;
	};

	std::unique_ptr<LoadedChunk> GenerateChunkMesh(const ChunkData& chunk, ChunkCoord coord, std::function<SerialBlock(BlockCoord)> blockDataFunc);
}