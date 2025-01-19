#pragma once

#include <Game/VoxelStuff/VoxelCube.h>
#include <Game/VoxelStuff/VoxelPlayer.h>

#include "ParkourShapes.h"


namespace Parkour
{
	class ParkourLightBlock : public Voxel::ICube
	{
	public:
		ParkourLightBlock(G1::IGSpace* container, CommonResources* resources, Voxel::VoxelWorld* world, Voxel::VoxelChunk* chunk, Voxel::ChunkBlockCoord pos);

		inline virtual void BeforeDraw() override {}
		inline virtual void AfterDraw() override {}

		virtual void OnLoaded() override;
		virtual void OnUnloaded() override;

		virtual std::unique_ptr<ICube> Clone(Voxel::VoxelWorld* world, Voxel::VoxelChunk* chunk, Voxel::ChunkBlockCoord pos) const override;
	protected:

	};

	class ParkourCheckpointBlock : public Voxel::ICube
	{
	public:
		ParkourCheckpointBlock(G1::IGSpace* container, CommonResources* resources, Voxel::VoxelWorld* world, Voxel::VoxelChunk* chunk, Voxel::ChunkBlockCoord pos);

		inline virtual void BeforeDraw() override {}
		virtual void AfterDraw() override;

		virtual std::unique_ptr<ICube> Clone(Voxel::VoxelWorld* world, Voxel::VoxelChunk* chunk, Voxel::ChunkBlockCoord pos) const override;
	protected:
		Pointer::f_ptr<Voxel::VoxelPlayer> m_Player;
		Pointer::f_ptr<PlayerTrackerShape> m_Tracker;
	};
}
