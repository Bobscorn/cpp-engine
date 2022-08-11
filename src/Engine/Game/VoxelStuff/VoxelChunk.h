#pragma once

#include "VoxelValues.h"

#include "Structure/BasicShapes.h"

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
}

namespace Voxel
{
	struct VoxelWorld;

	struct BlockData
	{
		size_t ID;
	};

	using ChunkBlockKey = ChunkCoord;

	typedef std::array<std::array<std::array<BlockData, Voxel::Chunk_Size>, Voxel::Chunk_Height>, Voxel::Chunk_Size> RawChunkData;
	typedef std::unordered_map<ChunkBlockKey, BlockData> RawChunkDataMap;

	RawChunkData ConvertMapToData(const RawChunkDataMap& m);

	typedef std::array<std::array<std::array<std::unique_ptr<Voxel::ICube>, Chunk_Size>, Chunk_Height>, Chunk_Size> ChunkData;

	struct LoadedChunk
	{
		ChunkCoord Coord;
		RawChunkData ChunkDat;
		std::unique_ptr<std::vector<floaty3>> PhysicsPositions;
		std::unique_ptr<std::vector<unsigned int>> PhysicsIndices;
		std::shared_ptr<btTriangleIndexVertexArray> PhysicsTriangles;
		std::shared_ptr<btBvhTriangleMeshShape> PhysicsShape;
		Drawing::RawMesh Mesh;
	};

	class ChunkyBoi : public G1::IShape, public BulletHelp::INothingInterface
	{
	public:

		ChunkyBoi(G1::IGSpace *container, CommonResources *resources, VoxelWorld *world, floaty3 origin, ChunkCoord coord);
		ChunkyBoi(G1::IGSpace *container, CommonResources *resources, VoxelWorld *world, floaty3 origin, RawChunkData initial_dat, ChunkCoord coord);
		ChunkyBoi(G1::IGSpace *container, CommonResources *resources, VoxelWorld *world, floaty3 origin, RawChunkDataMap initial_dat, ChunkCoord coord);
		ChunkyBoi(G1::IGSpace* container, CommonResources* resources, VoxelWorld* world, floaty3 origin, LoadedChunk preLoadedData);
		~ChunkyBoi();

		void BeforeDraw() override;
		void AfterDraw() override;

		void UpdateOrigin(floaty3 new_origin);

		ChunkyFrustumCuller *GetCuller() const;

		//inline operator std::array<std::array<std::array<std::unique_ptr<Voxel::ICube>, Chunk_Size>, Chunk_Height>, Chunk_Size> &const() { return m_Data; }
		
		void set(size_t x, size_t y, size_t z, std::unique_ptr<Voxel::ICube> val);
		void create(size_t x, size_t y, size_t z);

		ICube* get(size_t x, size_t y, size_t z);

		void SetTo(RawChunkData data);

		void SetFrom(LoadedChunk preLoadedChunk, bool constructCubes = true);

		void RecomputeMesh();

	protected:
		
		std::unique_ptr<ChunkyFrustumCuller> CreateCuller(floaty3 origin);

		ChunkData m_Data;
		std::vector<ChunkBlockKey> m_UpdateBlocks;
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

	LoadedChunk GenerateChunkMesh(const RawChunkData& chunk, ChunkCoord coord, std::function<size_t(BlockCoord)> isBlockFunc);
	LoadedChunk GenerateChunkMesh(const ChunkData& chunk, ChunkCoord coord, std::function<size_t(BlockCoord)> isBlockFunc);
}