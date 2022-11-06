#pragma once

#include "Helpers/GLHelper.h"

#include "Systems/Events/EventsBase.h"
#include "Systems/Threading/ThreadedQueue.h"

#include "VoxelChunk.h"
#include "Entities/VoxelProjectiles.h"

#include <unordered_set>
#include <climits>
#include <type_traits>
#include <thread>
#include <memory>
#include <atomic>
#include <shared_mutex>


namespace Voxel
{
	struct Entity;

	struct IWorldUpdater
	{
		virtual void DisplaceWorld(floaty3 by) = 0;
	};

	struct IChunkLoader
	{
		virtual RawChunkDataMap LoadChunk(int64_t x, int64_t y, int64_t z) = 0;
		inline RawChunkDataMap LoadChunk(ChunkCoord coord) { return LoadChunk(coord.X, coord.Y, coord.Z); }
	};

	struct IChunkUnloader
	{
		virtual void UnloadChunk(std::unique_ptr<VoxelChunk> chunk) = 0;
	};

	struct LoadingStuff
	{
		Threading::ThreadedQueue<ChunkCoord> ToLoad;
		Threading::ThreadedQueue<std::unique_ptr<LoadedChunk>> Loaded;
		std::atomic<bool> QuitVal;
	};

	struct LoadingOtherStuff
	{
		// Must be a thread-safe function that determines whether there is a block at specified position
		std::function<SerialBlock(BlockCoord coord)> GetBlockIdFunc;
		std::function<std::unique_ptr<ChunkData>(ChunkCoord coord)> GetChunkDataFunc;
	};

	void DoChunkLoading(std::shared_ptr<LoadingStuff> stuff, LoadingOtherStuff other);
	std::vector<ChunkCoord> CalculateOffsets(int64_t xHalfBound, int64_t yHalfBound, int64_t zHalfBound);

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
	struct VoxelWorld : G1::IShape, Events::IEventListenerT<Events::AfterPhysicsEvent>, IChunkUnloader
	{
		VoxelWorld(G1::IShapeThings things, WorldStuff stuff);
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
		void SetCube(BlockCoord coord, std::unique_ptr<ICube> cube);
		void SetCube(BlockCoord coord, const SerialBlock& block);
		void SetCube(BlockCoord coord, const NamedBlock& block);

		ICube* GetCubeAt(BlockCoord coord); // Get is thread-safe, modification via returned pointer not thread-safe
		const ICube* GetCubeAt(BlockCoord coord) const; // Thread-safe
		SerialBlock GetCubeDataAt(BlockCoord coord) const; // Thread-safe
		bool IsCubeAt(BlockCoord coord) const; // Thread-safe

		// Get the coords of a block/chunk given by position, in Displaced Physics space
		BlockCoord GetBlockCoordFromPhys(floaty3 phys_pos);
		ChunkCoord GetChunkCoordFromPhys(floaty3 phys_pos);
		floaty3 GetPhysPosFromBlockCoord(BlockCoord coord);

		// Dynamic Stuff
		void AddEntity(std::unique_ptr<Entity> entity);
		void RemoveEntity(Entity *entity);

		template <class proj_type, class ... Args>
		void AddProjectile(Args ... args);

		template<>
		void AddProjectile<RayProjectile, floaty3, floaty3, float, Entity *, DamageDescription>(floaty3, floaty3, float, Entity *, DamageDescription);
		template<>
		void AddProjectile<HitScanProjectile, floaty3, floaty3, Entity *, DamageDescription>(floaty3, floaty3, Entity *, DamageDescription);


		// Chunk Unloading
		void UnloadChunk(std::unique_ptr<VoxelChunk> chunk) override;

	protected:

		WorldStuff m_Stuff;
		std::shared_ptr<LoadingStuff> m_LoadingStuff;
		std::thread m_LoadingThread;
		mutable std::shared_mutex m_ChunksMutex; // Synchronise access to chunks to allow for reading from a separate loading thread

		// Store a list of pre-calculated offsets from the centre/player to load as the player/centre moves
		std::vector<ChunkCoord> m_ChunkLoadingOffsets;

		// Store the actual chunks in an unordered_map
		// Storing them as unique_ptrs is perhaps not necessary
		std::unordered_map<ChunkCoord, std::unique_ptr<VoxelChunk>> m_Chunks;
		
		// Temporary measure to store changes and prevent them being unloaded
		std::unordered_map<ChunkCoord, std::vector<std::pair<ChunkBlockCoord, std::unique_ptr<ICube>>>> m_UpdateBlockChanges;
		std::unordered_map<ChunkCoord, std::vector<std::pair<ChunkBlockCoord, SerialBlock>>> m_BlockChanges;

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

		void ApplyChunkChanges();

		void CheckLoadingThread();

		// Begins loading chunk at specific coord
		void Load(ChunkCoord at);
		void Unload(ChunkCoord at);

		floaty3 ChunkOrigin(ChunkCoord of);

		void DoRemoveEntities();
	};
}