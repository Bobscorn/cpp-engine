#pragma once

#include "Helpers/GLHelper.h"

#include "Systems/Events/EventsBase.h"

#include "VoxelChunk.h"
#include "Entities/VoxelProjectiles.h"

#include <unordered_set>
#include <climits>
#include <type_traits>


namespace Voxel
{
	struct Entity;

	struct IWorldUpdater
	{
		virtual void DisplaceWorld(floaty3 by) = 0;
	};

	struct IChunkLoader
	{
		virtual ChunkyBoi::RawChunkDataMap LoadChunk(int64_t x, int64_t y, int64_t z) = 0;
	};

	struct IChunkUnloader
	{
		virtual void UnloadChunk(std::unique_ptr<ChunkyBoi> &&chunk) = 0;
	};

	struct WorldStuff
	{
		IWorldUpdater *m_WorldUpdater;
		IChunkLoader *m_ChunkLoader;
		IChunkUnloader *m_ChunkUnloader;
		// The half extent of extra chunks along the X/Y/Z Axes, there will always be the very centre chunk
		size_t HalfBonusWidth = 4;
		size_t HalfBonusHeight = 1;
		size_t HalfBonusDepth = 4;

		size_t ChunkLeniance = 1; // The number of chunks the centre must be away from the middle loaded chunk to load/unload new/old chunks
	};

	// VoxelWorld is a class designed to load*, unload chunks, and displace the physics world in order to keep the player at the centre of world
	// *Loading is not done in this class
	struct VoxelWorld : G1::IShape, Events::IEventListenerT<Events::AfterPhysicsEvent>
	{
		VoxelWorld(G1::IGSpace *container, CommonResources *resources, WorldStuff stuff);
		~VoxelWorld();

		// Update is the main method of the dynamic world centre
		// It takes an input of New_Centre, a vector describing a new Centre of the World (usually the player), in Displaced World Space ie. New_Centre must treat m_PhysicsDisplacement as its origin
		// It returns the given (if any) displacement of the physics world allowing an centre of world not updated via m_WorldUpdater->DisplaceWorld to keep track of the new origin
		floaty3 Update(floaty3 New_Centre);

		void BeforeDraw() override;
		void Draw() override;
		void AfterDraw() override;

		bool Receive(Events::IEvent *e) override;
		bool Receive(::Event::AfterPhysicsEvent *event) override;

		// Static Modification
		void ReplaceStaticCube(BlockCoord coord, std::unique_ptr<ICube> cube);
		void SetStaticCube(BlockCoord coord);

		ICube* GetCubeAt(BlockCoord coord);
		bool IsCubeAt(BlockCoord coord);

		// Get the coords of a block/chunk given by position, in Displaced Physics space
		BlockCoord GetBlockCoordFromPhys(floaty3 phys_pos);
		ChunkCoord GetChunkCoordFromPhys(floaty3 phys_pos);

		// Dynamic Stuff
		void AddEntity(std::unique_ptr<Entity> entity);
		void RemoveEntity(Entity *entity);

		template <class proj_type, class ... Args>
		void AddProjectile(Args ... args);

		template<>
		void AddProjectile<RayProjectile, floaty3, floaty3, float, Entity *, DamageDescription>(floaty3, floaty3, float, Entity *, DamageDescription);
		template<>
		void AddProjectile<HitScanProjectile, floaty3, floaty3, Entity *, DamageDescription>(floaty3, floaty3, Entity *, DamageDescription);

	protected:

		WorldStuff m_Stuff;


		std::unordered_map<ChunkCoord, std::unique_ptr<ChunkyBoi>> m_Chunks;

		std::unordered_map<Entity *, std::unique_ptr<Entity>> m_DynamicEntities;
		std::vector<Entity *> m_ToRemoveEntities;

		std::vector<RayProjectile> m_RayProjectiles;
		std::vector<HitScanProjectile> m_HitscanProjectiles;


		// Displacement for the entire physics world
		DOUBLE3 m_PhysicsDisplacement = { 0.0, 0.0, 0.0 };

		GLProgram m_ProjectileProgram;
		GLBuffer m_ProjectileVBO;
		GLBuffer m_ProjectileIBO;
		GLBuffer m_ProjectileInstanceBuf;
		GLBuffer m_ProjectilePerObjectBuf;
		GLVertexArray m_ProjectileVAO;
		size_t m_InstanceBufferSize = 0;

		struct ProjInstanceData
		{
			floaty3 ProjectilePos;
			floaty3 Color;
			Matrixy3x3 ProjectileScaleRotation;
		};

		GLuint CreateVAO();
		GLuint CreateVBO();
		GLuint CreateIBO();
		void UpdateInstanceStuff(const std::vector<ProjInstanceData> &data);




		void Load(ChunkCoord at);

		floaty3 ChunkOrigin(ChunkCoord of);

		void DoRemoveEntities();
	};
}