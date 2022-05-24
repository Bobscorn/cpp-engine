#pragma once

#include "VoxelValues.h"

#include "Structure/BasicShapes.h"

#include "VoxelTypes.h"
#include "VoxelCube.h"
#include "VoxelChunkCuller.h"

#include "Drawing/Mesh.h"
#include "Drawing/Material.h"
#include "Drawing/DrawCallReference.h"

#include <BulletCollision/CollisionShapes/btBoxShape.h>

#include <vector>
#include <memory>
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

	class ChunkyBoi : public G1::IShape
	{
	public:

		typedef std::array<std::array<std::array<BlockData, Voxel::Chunk_Size>, Voxel::Chunk_Height>, Voxel::Chunk_Size> RawChunkData;
		typedef std::unordered_map<ChunkCoord, BlockData> RawChunkDataMap;

		ChunkyBoi(G1::IGSpace *container, CommonResources *resources, VoxelWorld *world, floaty3 origin, ChunkCoord coord);
		ChunkyBoi(G1::IGSpace *container, CommonResources *resources, VoxelWorld *world, floaty3 origin, RawChunkData initial_dat, ChunkCoord coord);
		ChunkyBoi(G1::IGSpace *container, CommonResources *resources, VoxelWorld *world, floaty3 origin, RawChunkDataMap initial_dat, ChunkCoord coord);
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

		typedef std::array<std::array<std::array<std::unique_ptr<Voxel::ICube>, Chunk_Size>, Chunk_Height>, Chunk_Size> ChunkData;

		void RecomputeMesh();

	protected:
		
		std::unique_ptr<ChunkyFrustumCuller> CreateCuller(floaty3 origin);

		ChunkData m_Data;
		floaty3 m_Origin;
		VoxelWorld *m_World = nullptr;
		ChunkCoord m_Coord;

		std::unique_ptr<ChunkyFrustumCuller> m_Culler;

		std::shared_ptr<btBoxShape> m_Shape;
		std::shared_ptr<Drawing::Material> m_Material;

		// v2 Rendering stuff
		std::shared_ptr<Drawing::Mesh> m_Mesh;
		std::shared_ptr<Drawing::Material> m_Matv2;
		std::shared_ptr<Matrixy4x4> m_Matrix;

		Drawing::DrawCallReference m_DrawCall;
	};
}