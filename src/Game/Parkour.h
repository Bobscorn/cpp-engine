#pragma once

#include <Helpers/VectorHelper.h>

#include <Game/VoxelStuff/VoxelTypes.h>

#include <vector>
#include <unordered_map>

namespace Parkour
{

	// A looking point is an block and a direction (a face) on that block
	struct LookingPoint
	{
		Vector::inty3 Position;
		Voxel::BlockFace Direction;
	};

	constexpr Voxel::BlockFace ParkourForwardDirection = Voxel::BlockFace::Forward;
	constexpr Voxel::BlockFace ParkourRightDirection = Voxel::BlockFace::Right;
	constexpr Voxel::BlockFace ParkourLeftDirection = Voxel::BlockFace::Left;
	constexpr Voxel::BlockFace ParkourBackDirection = Voxel::BlockFace::Back;
	constexpr Voxel::BlockFace ParkourUpDirection = Voxel::BlockFace::Up;
	constexpr Voxel::BlockFace ParkourDownDirection = Voxel::BlockFace::Down;

	/// <summary>
	/// A parkourstep is a plug-in-able group of blocks that make up one singular obstacle or 'step' in a parkour level
	/// Each parkourstep is assumed to start at 0,0,0
	/// Each parkoutstep is assumed to be facing in ParkourForwardDirection direction (which is -Z/Forward by default)
	/// This means this step's blocks are assumed to extend in the Forward/-Z direction.
	/// Each step's block will be rotated as needed.
	/// Each exit/LookingPoint's position is relative to 0,0,0 and direction is relative to ParkourForwardDirection/-Z/Forward
	/// Each exit's Position must be a block that is part of this step
	/// 
	/// Each step is required to have at least one exit.
	/// </summary>
	struct ParkourStep
	{
		ParkourStep() {}
		ParkourStep(std::vector<std::pair<Voxel::NamedBlock, Vector::inty3>> blocks, std::vector<LookingPoint> exits) : Blocks(blocks), Exits(exits) {}

		std::vector<std::pair<Voxel::NamedBlock, Vector::inty3>> Blocks;

		std::vector<LookingPoint> Exits;
	};

	struct ParkourLevel
	{
		Voxel::BlockCoord StartPos;
		Voxel::BlockFace StartFace;
		std::vector<ParkourStep> UsableSteps;
		std::vector<std::pair<float, int>> WeightedDistances;
		std::vector<std::pair<float, int>> WeightedAltitudes;
		size_t NumGeneratedSteps;
		Voxel::BlockCoord EndPosition;
		Voxel::BlockFace EndFace;
		Voxel::BlockCoord GoalPosition;
	};

	/// <summary>
	/// Struct used for output of the GenerateParkour function
	/// The Blocks unordered_map is a map of all the blocks in the parkour level relative to origin of StartPosition passed in the ParkourGenerationInfo
	/// The EndPosition is the position of the last exit picked in generation + 1 in the end direction, or 0,0,0 if no exit was picked.
	/// A block should not exist at EndPosition, but a faulty step, negative distance, or the parkour looping on itself among other things may cause this.
	/// The EndDirection is the direction of the last exit picked in generation, or ParkourForwardDirection if no exit was picked.
	/// 
	/// An exit should always be picked unless: No *valid* steps were supplied, or the number of steps generated was 0.
	/// </summary>
	struct GeneratedParkour
	{
		GeneratedParkour() : Blocks(), EndPosition(0, 0, 0), EndDirection(ParkourForwardDirection) {}
		std::unordered_map<Vector::inty3, Voxel::NamedBlock> Blocks;
		Vector::inty3 EndPosition;
		Voxel::BlockFace EndDirection;
	};

	struct ParkourGenerationInfo
	{
		ParkourGenerationInfo() : StartDirection(ParkourForwardDirection), StartPosition(0, 0, 0), UsableSteps(), WeightedDistances(), WeightedAltitudes(), NumGeneratedSteps(0) {};
		Voxel::BlockFace StartDirection;
		Vector::inty3 StartPosition;
		std::vector<ParkourStep> UsableSteps;
		std::vector<std::pair<float, int>> WeightedDistances;
		std::vector<std::pair<float, int>> WeightedAltitudes;
		size_t NumGeneratedSteps;
	};

	GeneratedParkour GenerateParkour(const ParkourGenerationInfo& other);

	/// <summary>
	/// Convert an inty3 position into a BlockCoord position usable in a voxel world
	/// </summary>
	/// <param name="pos">The incoming inty3 position</param>
	/// <param name="origin">The base origin the inty3 position is relative to</param>
	/// <param name="rotation">The rotation about the origin the position has</param>
	/// <returns>The BlockCoord represented the inty3 position rotated and translated about the origin</returns>
	Voxel::BlockCoord inty3ToBlockCoord(Vector::inty3 pos, Voxel::BlockCoord origin = Voxel::BlockCoord::Origin(), quat4 rotation = quat4::identity());
	Vector::inty3 BlockCoordTointy3(Voxel::BlockCoord pos);
		
}

