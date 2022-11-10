#pragma once

#include "Drawing/Graphics1.h"
#include "Drawing/Graphics3D.h"

#include "VoxelChunkCuller.h"
#include "VoxelTypes.h"

#include "Helpers/BulletHelper.h"
#include "Helpers/VectorHelper.h"

#include <memory>

namespace Voxel
{
	class VoxelWorld;
	class VoxelChunk;

	struct CubeCreationDescription
	{
		std::vector<std::string> MaterialNames;
		std::vector<std::string> GeometryNames;
	};

	struct CubeCreationStuff
	{
	};

	struct ICube : virtual G1::IShape
	{
		ICube(VoxelWorld *world, Voxel::VoxelChunk* chunk, Voxel::ChunkBlockCoord pos) : m_World(world), m_Chunk(chunk), m_Pos(pos) {}

		inline VoxelWorld* GetWorld() const { return m_World; }
		inline VoxelChunk* GetChunk() const { return m_Chunk; }
		inline ChunkBlockCoord GetChunkPos() const { return m_Pos; }
		BlockCoord GetWorldPos() const;
		const std::string& GetBlockName() const;
		CubeID GetBlockID() const;
		const SerialBlock& GetBlockData() const;
		SerialBlock TakeBlockData();

		virtual std::unique_ptr<ICube> Clone(VoxelWorld* world, VoxelChunk* chunk, ChunkBlockCoord pos) const = 0;
		inline virtual bool WantsUpdate() const { return false; }

		// OnPlaced is called when this block is placed into the world
		inline virtual void OnPlaced() {}
		// OnDestroyed is called when this block is destroyed (not unloaded)
		inline virtual void OnDestroyed() {}
		// OnLoaded is called when this block is loaded into a chunk (this includes when it is placed)
		inline virtual void OnLoaded() {}
		// OnUnloaded is called when this block is unloaded from a chunk (this includes when it is destroyed)
		inline virtual void OnUnloaded() {}

		inline void Attach(VoxelWorld* world, VoxelChunk* chunk, ChunkBlockCoord pos) { m_World = world; m_Chunk = chunk; m_Pos = pos; }
		inline void Detach(SerialBlock data) { m_Chunk = nullptr; m_World = nullptr; if (!m_Data) m_Data = std::make_unique<SerialBlock>(); *m_Data = std::move(data); }

	protected:
		VoxelWorld* m_World = nullptr;
		Voxel::VoxelChunk* m_Chunk = nullptr;
		Voxel::ChunkBlockCoord m_Pos;
		std::unique_ptr<SerialBlock> m_Data;
	};

	struct VoxelCube : ICube, virtual G1::IShape, BulletHelp::INothingInterface
	{
		VoxelCube(G1::IGSpace *container, CommonResources *resources, VoxelWorld *world, VoxelChunk* chunk, ChunkBlockCoord coord);
		~VoxelCube();

		virtual void BeforeDraw() override;
		virtual void AfterDraw() override;

		void SetCull(ChunkyFrustumCuller *chunkyboi);

		virtual std::unique_ptr<ICube> Clone(VoxelWorld* world, VoxelChunk* chunk, ChunkBlockCoord pos) const override;

	protected:

		void ResetCuller();

		//std::shared_ptr<btBoxShape> m_Shape;
		//static std::weak_ptr<btBoxShape> s_Shape;

		//std::shared_ptr<btCollisionObject> m_Body;
	};

	/*struct VoxelCubeKeptForTemplate : ICube, virtual G1::IShape, BulletHelp::INothingInterface
	{
		VoxelCubeKeptForTemplate(G1::IGSpace *container, CommonResources *resources, VoxelWorld *world, floaty3 position, size_t x, size_t y, size_t z, ChunkyFrustumCuller *chunkcull = nullptr);
		~VoxelCubeKeptForTemplate();

		virtual void BeforeDraw() override;
		virtual void AfterDraw() override;

		virtual std::shared_ptr<GeoThing> GetGeo() override;

		floaty3 GetPosition() const;

		void SetCull(ChunkyFrustumCuller *chunkyboi);
		void SetChunkPosition(size_t x, size_t y, size_t z);

		virtual void UpdatePosition(floaty3 new_position, size_t new_x, size_t new_y, size_t new_z, ChunkyFrustumCuller *new_culler) override;

	protected:

		void ResetCuller();
		
		std::shared_ptr<Material> GetMat();
		std::shared_ptr<btBoxShape> GetShape();
		std::shared_ptr<btCollisionObject> GetBody(btCollisionShape *shape);

		Matrixy4x4 m_Trans;
		size_t m_X = 0, m_Y = 0, m_Z = 0;
		size_t m_DrawKey = 0ull;

		std::unique_ptr<BlockCuller> m_Culler;

		std::shared_ptr<GeoThing> m_Geo;
		std::shared_ptr<Material> m_Mat;
		std::shared_ptr<btBoxShape> m_Shape;
		static std::weak_ptr<GeoThing> s_Geo;
		static std::weak_ptr<Material> s_Mats[5];
		static std::weak_ptr<btBoxShape> s_Shape;

		std::shared_ptr<btCollisionObject> m_Body;

		constexpr static Material s_MaterialDatas[5] = 
		{
			Material({ 0.05f, 0.05f, 0.05f, 1.f }, { 0.f, 0.f, 0.f, 0.f }, { 0.8f, 0.8f, 0.8f, 1.f }, { 0.3f, 0.3f, 0.3f, 25.f }, 1, 0),
			Material({ 0.05f, 0.05f, 0.05f, 1.f }, { 0.f, 0.f, 0.f, 0.f }, { 0.5f, 0.5f, 0.5f, 1.f }, { 0.25f, 0.25f, 0.25f, 25.f }, 1, 0),
			Material({ 0.05f, 0.05f, 0.05f, 1.f }, { 0.f, 0.f, 0.f, 0.f }, { 0.8f, 0.5f, 0.5f, 1.f }, { 0.2f, 0.35f, 0.35f, 25.f }, 1, 0), 
			Material({ 0.05f, 0.05f, 0.05f, 1.f }, { 0.f, 0.f, 0.f, 0.f }, { 0.5f, 0.8f, 0.5f, 1.f }, { 0.35f, 0.2f, 0.35f, 25.f }, 1, 0),
			Material({ 0.05f, 0.05f, 0.05f, 1.f }, { 0.f, 0.f, 0.f, 0.f }, { 0.5f, 0.5f, 0.8f, 1.f }, { 0.3f, 0.3f, 0.3f, 25.f }, 1, 0) 
		};
	};*/
}