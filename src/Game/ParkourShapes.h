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

	// A PlayerTorchShape
	// refs the Player
	// Updates the player's 'torch' to match where they are looking

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

	// A shape that will generate parkour upon creation
	class ParkourGeneratorShape : public virtual G1::IShape
	{
	public:
		ParkourGeneratorShape(G1::IShapeThings things, Pointer::f_ptr<ParkourLevelShape> levelShape, Pointer::f_ptr<Voxel::VoxelWorld> worldShape, bool generate);

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

	// Tracks player position for specific conditions like being at the goal
	class PlayerTrackerShape : public virtual G1::IShape
	{
	public:
		PlayerTrackerShape(G1::IShapeThings things, PlayerTrackingData data);
		
		// Inherited via IShape
		virtual void BeforeDraw() override;
		virtual void AfterDraw() override;

		inline void SetRespawnPosition(floaty3 position) { m_RespawnOverride = std::make_unique<floaty3>(position); }
		inline const floaty3 const* GetRespawnPosition() const { return m_RespawnOverride.get(); }

		inline void Reset() { m_HasSentWinRequest = false; m_RespawnOverride = nullptr; }
	protected:
		PlayerTrackingData m_Data;
		std::unique_ptr<floaty3> m_RespawnOverride;
		bool m_HasSentWinRequest = false;
	};

	// A visual shape used to indicate where the end of parkour is
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


	// Testing shape to record fps
	class ParkourTimeMeasuringShape : public virtual G1::IShape, public Requests::IRequestable
	{
	public:
		ParkourTimeMeasuringShape(G1::IShapeThings things);

		virtual void BeforeDraw() override;
		inline virtual void AfterDraw() override {}

		virtual Debug::DebugReturn Request(Requests::Request& req) override;
		inline virtual std::string GetName() const override { return "ParkourTimeMeasurer"; }
	protected:
		bool m_Recording = false;
		unsigned int m_RunNumber = 0;
		float m_StartTime = 0.f;
		unsigned int m_StartUpdateID = 0;
		float m_Period = 10.f;
		unsigned int m_NumRuns = 5;
	};
}
