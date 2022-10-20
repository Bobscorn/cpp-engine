#pragma once

#include <Drawing/Graphics1.h>
#include <Drawing/GLRen2.h>
#include <Game/VoxelStuff/VoxelWorld.h>
#include <Game/VoxelStuff/VoxelPlayer.h>

#include <Math/matrix.h>

#include "Parkour.h"

#include <memory>

namespace Parkour
{
	// Shapes v

	// A ParkourLevel shape
	// Contains all relevant stuff for a specific level

	// A ParkourGenerator shape
	// refs ParkourLevel shape
	// Reads ParkourLevel shape and generates the blocks for the level

	// A PlayerTrackerShape
	// refs ParkourLevel shape
	// Track's player's position, checking they haven't fallen, or they are at the end.

	// A ParkourEndShape
	// refs ParkourLevel shape
	// Visually displays the end of the current parkour level
	// No behaviour except for displaying the visuals

	class ParkourLevelShape : public virtual G1::IShape
	{
	public:
		ParkourLevelShape(G1::IShapeThings things, ParkourLevel level);

		// Inherited via IShape
		virtual void BeforeDraw() override;
		virtual void AfterDraw() override;

		const ParkourLevel& GetLevelData() const;
		void SetLevelData(ParkourLevel level);

	protected:
		ParkourLevel m_Level;
	};

	class ParkourGeneratorShape : public virtual G1::IShape
	{
	public:
		ParkourGeneratorShape(G1::IShapeThings things, Pointer::f_ptr<ParkourLevelShape> levelShape, Pointer::f_ptr<Voxel::VoxelWorld> worldShape);

		inline virtual void BeforeDraw() override {}
		inline virtual void AfterDraw() override {}

		void Generate();
	protected:
		Pointer::f_ptr<ParkourLevelShape> m_LevelShape;
		Pointer::f_ptr<Voxel::VoxelWorld> m_WorldShape;
	};

	struct PlayerTrackingData
	{
		Pointer::f_ptr<Voxel::VoxelPlayer> Player;
		Pointer::f_ptr<Voxel::VoxelWorld> World;
		Pointer::f_ptr<ParkourLevelShape> Level;
		
		float GoalDistanceSq;
		float MinimumY;
	};

	class PlayerTrackerShape : public virtual G1::IShape
	{
	public:
		PlayerTrackerShape(G1::IShapeThings things, PlayerTrackingData data);
		
		// Inherited via IShape
		virtual void BeforeDraw() override;
		virtual void AfterDraw() override;
	protected:
		PlayerTrackingData m_Data;
	};

	class ParkourEndShape : public virtual G1::IShape
	{
	public:
		ParkourEndShape(G1::IShapeThings things, Pointer::f_ptr<ParkourLevelShape> levelShape, Pointer::f_ptr<Voxel::VoxelWorld> worldShape);

		// Inherited via IShape
		virtual void BeforeDraw() override;
		inline virtual void AfterDraw() override {}
	protected:
		Pointer::f_ptr<ParkourLevelShape> m_LevelShape;
		Pointer::f_ptr<Voxel::VoxelWorld> m_WorldShape;
		
		std::shared_ptr<Matrixy4x4> m_DrawTrans;
		Drawing::DrawCallReference m_DrawCall;

		float m_RotationDegrees;
		float m_RotationDegreesPerSec;
	};

}
