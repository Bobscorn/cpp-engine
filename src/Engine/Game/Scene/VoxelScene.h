#pragma once

#include "Systems/Execution/Scene.h"

#include "Helpers/TransferHelper.h"

#include "Structure/BasicGSpace.h"

#include "Game/VoxelStuff/VoxelPlayer.h"
#include "Game/VoxelStuff/VoxelChunk.h"
#include "Game/VoxelStuff/VoxelWorld.h"
#include "Game/VoxelStuff/VoxelMemoryLevel.h"

#include "Game/VoxelStuff/VoxelPlayerUI.h"

namespace Voxel
{
	struct DefaultWorldLoader : Voxel::IWorldUpdater, Voxel::IChunkLoader, Voxel::IChunkUnloader
	{
		// Inherited via IWorldUpdater
		virtual void DisplaceWorld(floaty3 by) override;

		// Inherited via IChunkLoader
		virtual RawChunkDataMap LoadChunk(int64_t x, int64_t y, int64_t z) override;

		// Inherited via IChunkUnloader
		virtual void UnloadChunk(std::unique_ptr<VoxelChunk> chunk) override;
	};

	class VoxelScene : FullResourceHolder, public Scene::IScene
	{
	public:
		VoxelScene(CommonResources *resources);
		~VoxelScene();

		Debug::DebugReturn Initialize() override;

		bool Receive(Events::IEvent *event) override;

		void BeforeDraw() override;
		void Draw() override;
		void AfterDraw() override;

		std::unique_ptr<IScene> Clone() override;

	protected:

		// G Space
		G1I::BasicGSpace m_GSpace;

		// The world
		Pointer::f_ptr<Voxel::VoxelWorld> m_World;
		DefaultWorldLoader m_Loader;
		VoxelMemoryLevel m_ChunkMemory;

		// Player
		Pointer::f_ptr<Voxel::VoxelPlayer> m_Player;

		UI1::RootElement m_UI;
		Voxel::Crosshair m_Crosshair;
	};
}