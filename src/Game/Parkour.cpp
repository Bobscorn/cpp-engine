#include "Parkour.h"

#include <random>

#include "Helpers/DebugHelper.h"

namespace Parkour
{
	GeneratedParkour GenerateParkour(const ParkourGenerationInfo& genInfo)
	{
		using namespace Vector;

		if (genInfo.UsableSteps.empty())
		{
			DERROR("Can not generate parkour with 0 usable steps!");
			return GeneratedParkour{};
		}

		std::vector<ParkourStep> realSteps{};
		realSteps.reserve(genInfo.UsableSteps.size());
		for (const ParkourStep& step : genInfo.UsableSteps)
		{
			if (!step.Exits.empty())
			{
				realSteps.push_back(step);
			}
		}

		if (realSteps.empty())
		{
			DERROR("Can not generate parkour with 0 usable steps! (Some steps had no exits!)");
			return GeneratedParkour{};
		}

		GeneratedParkour parkour;

		// Initialize Random stuff
		float totalDistanceWeights = 0.f;
		for (auto& distance : genInfo.WeightedDistances)
		{
			totalDistanceWeights += distance.first;
		}
		float totalAltitudeWeights = 0.f;
		for (auto& altitude : genInfo.WeightedAltitudes)
		{
			totalAltitudeWeights += altitude.first;
		}

		std::default_random_engine generator;
		generator.seed(std::random_device()());
		std::uniform_int_distribution<size_t> stepDistribution(0, realSteps.size() - 1);
		std::uniform_real_distribution<float> distanceDistribution(0, totalDistanceWeights);
		std::uniform_real_distribution<float> altitudeDistribution(0, totalAltitudeWeights);

		auto pickStep = [&generator, &stepDistribution](const std::vector<ParkourStep>& availableSteps)
		{
			return availableSteps[stepDistribution(generator)];
		};

		auto pickExit = [&generator](const std::vector<LookingPoint>& exits)
		{
			if (exits.empty())
				return LookingPoint{};
			return exits[std::uniform_int_distribution<size_t>(0, exits.size() - 1)(generator)];
		};

		auto pickDistance = [&generator, &distanceDistribution](const std::vector<std::pair<float, int>>& availableDistances)
		{
			if (availableDistances.empty())
				return 0;
			auto val = distanceDistribution(generator);
			for (auto& distance : availableDistances)
			{
				if (val < distance.first)
				{
					return distance.second;
				}
				val -= distance.first;
			}
			throw std::runtime_error("Failed to pick distance!");
		};

		auto pickAltitude = [&generator, &altitudeDistribution](const std::vector<std::pair<float, int>>& availableAltitudes)
		{
			if (availableAltitudes.empty())
				return 0;
			auto val = altitudeDistribution(generator);
			for (auto& altitude : availableAltitudes)
			{
				if (val < altitude.first)
				{
					return altitude.second;
				}
				val -= altitude.first;
			}
			throw std::runtime_error("Failed to pick altitude!");
		};

		inty3 currentPos = genInfo.StartPosition;
		quat4 currentRotation = Voxel::GetWhatRotates(ParkourForwardDirection, genInfo.StartDirection);
		inty3 nextDistance = inty3(0, 0, 0);
		for (auto i = 0; i < genInfo.NumGeneratedSteps; ++i)
		{
			currentPos += nextDistance;

			auto step = pickStep(realSteps);
			for (auto& block : step.Blocks)
			{
				auto toAdd = block.first;
				toAdd.Data.Rotation = currentRotation * toAdd.Data.Rotation;

				auto pos = block.second;

				inty3 realPos;
				{
					floaty3 posf = floaty3{ (float)pos.x, (float)pos.y, (float)pos.z };
					floaty3 rotatedPos = currentRotation.rotate(posf);
					realPos = inty3{ std::lroundf(rotatedPos.x), std::lroundf(rotatedPos.y), std::lroundf(rotatedPos.z) };
					realPos += currentPos;
				}

				parkour.Blocks[realPos] = toAdd;
			}

			auto exit = pickExit(step.Exits);

			auto distance = pickDistance(genInfo.WeightedDistances);
			auto altitude = pickAltitude(genInfo.WeightedAltitudes);

			auto dir = Voxel::BlockFaceHelper::GetDirectionI(Voxel::RotateFace(exit.Direction, currentRotation));

			nextDistance = dir * distance;
			inty3 rotatedExitPos;
			{
				floaty3 posf = floaty3{ (float)exit.Position.x, (float)exit.Position.y, (float)exit.Position.z };
				floaty3 rotatedPos = currentRotation.rotate(posf);
				rotatedExitPos = inty3{ std::lroundf(rotatedPos.x), std::lroundf(rotatedPos.y), std::lroundf(rotatedPos.z) };
			}
			currentPos += rotatedExitPos + dir * 1;
			currentPos.y += altitude;
			currentRotation *= Voxel::GetWhatRotates(ParkourForwardDirection, exit.Direction);
		}

		parkour.EndPosition = currentPos;
		parkour.EndDirection = Voxel::RotateFace(ParkourForwardDirection, currentRotation);

		return parkour;
	}

	Voxel::BlockCoord inty3ToBlockCoord(Vector::inty3 pos, Voxel::BlockCoord origin, quat4 rotation)
	{
		using Voxel::Chunk_Size;
		using Voxel::Chunk_Height;

		auto originPos = BlockCoordTointy3(origin);

		auto rotated = rotation.rotate(pos);

		pos = rotated + originPos;

		Vector::inty3 remainder;
		remainder.x = pos.x % (int)Chunk_Size;
		remainder.y = pos.y % (int)Chunk_Height;
		remainder.z = pos.z % (int)Chunk_Size;

		if (pos.x < 0)
			pos.x -= Chunk_Size - 1;
		if (pos.y < 0)
			pos.y -= Chunk_Height - 1;
		if (pos.z < 0)
			pos.z -= Chunk_Size - 1;

		Voxel::BlockCoord out;
		out.Chunk.X = pos.x / (int)Chunk_Size;
		out.Chunk.Y = pos.y / (int)Chunk_Height;
		out.Chunk.Z = pos.z / (int)Chunk_Size;

		out.Block.x = (size_t)(remainder.x < 0 ? (int)Chunk_Size + remainder.x : remainder.x);
		out.Block.y = (size_t)(remainder.y < 0 ? (int)Chunk_Height + remainder.y : remainder.y);
		out.Block.z = (size_t)(remainder.z < 0 ? (int)Chunk_Size + remainder.z : remainder.z);

		return out;
	}

	Vector::inty3 BlockCoordTointy3(Voxel::BlockCoord pos)
	{
		Vector::inty3 out;
		out.x = (int)(pos.Chunk.X * (int64_t)Voxel::Chunk_Size);
		out.y = (int)(pos.Chunk.Y * (int64_t)Voxel::Chunk_Height);
		out.z = (int)(pos.Chunk.Z * (int64_t)Voxel::Chunk_Size);
		out.x += (int)pos.Block.x;
		out.y += (int)pos.Block.y;
		out.z += (int)pos.Block.z;
		return out;
	}
}


#ifdef CPP_GAME_TESTS

#include <gtest/gtest.h>
#include <algorithm>

TEST(GenerateParkourTests, BasicAssertions)
{
	using namespace Vector;
	using namespace Voxel;
	using namespace Parkour;

	// Test inty3
	{
		EXPECT_EQ(inty3(), inty3(0, 0, 0));
		EXPECT_EQ(inty3().x, 0);
		EXPECT_EQ(inty3().y, 0);
		EXPECT_EQ(inty3().z, 0);
	}

	// Test Blank Output
	{
		ParkourGenerationInfo info;
		info.UsableSteps = {};
		info.WeightedAltitudes = {};
		info.WeightedDistances = {};
		info.NumGeneratedSteps = 0;
		info.StartDirection = ParkourForwardDirection;
		info.StartPosition = inty3();

		auto parkour = GenerateParkour(info);

		EXPECT_TRUE(parkour.Blocks.empty());
		EXPECT_EQ(parkour.EndDirection, ParkourForwardDirection);
		EXPECT_EQ(parkour.EndPosition, inty3());
	}

	// Test Constructor Output
	{
		ParkourGenerationInfo info;

		auto parkour = GenerateParkour(info);

		EXPECT_TRUE(parkour.Blocks.empty());
		EXPECT_EQ(parkour.EndDirection, ParkourForwardDirection);
		EXPECT_EQ(parkour.EndPosition, inty3());
	}
}

TEST(GenerateParkourTests, SimpleGeneration)
{
	using namespace Voxel;
	using namespace Vector;
	using namespace Parkour;

	// Test a single empty step
	// This should be identical to zero step output
	// We don't want a step without any endings to be used
	{
		ParkourGenerationInfo info;
		ParkourStep step;
		step.Blocks.emplace_back(std::make_pair(Voxel::NamedBlock{ "wood", Voxel::CubeData{ quat4() } }, Vector::inty3(0, 0, 0)));
		step.Exits.clear();
		
		info.UsableSteps.push_back(step);
		info.NumGeneratedSteps = 1;
		
		auto parkour = GenerateParkour(info);

		EXPECT_EQ(parkour.Blocks.size(), 0);
		EXPECT_EQ(parkour.EndPosition, inty3());
		EXPECT_EQ(parkour.EndDirection, ParkourForwardDirection);
	}

	// Test a single simple step
	// Should output the single step by itself
	{
		ParkourGenerationInfo info;
		ParkourStep step;
		step.Blocks.emplace_back(std::make_pair(Voxel::NamedBlock{ "wood", Voxel::CubeData{ quat4::identity() } }, Vector::inty3(0, 0, 0)));
		step.Exits.emplace_back(LookingPoint{ inty3(), ParkourForwardDirection});
		
		info.UsableSteps.push_back(step);
		info.NumGeneratedSteps = 1;
		
		const auto parkour = GenerateParkour(info);

		EXPECT_EQ(parkour.Blocks.size(), 1);
		EXPECT_EQ(parkour.Blocks.count(inty3()), 1);
		auto it = parkour.Blocks.find(inty3());
		EXPECT_TRUE(it != parkour.Blocks.end());
		EXPECT_EQ(it->second.Data.Rotation, quat4::identity());
		EXPECT_EQ(parkour.EndPosition, inty3(0, 0, -1));
		EXPECT_EQ(parkour.EndDirection, ParkourForwardDirection);
	}

	// Test a single simple step with a different ending direction
	// Should output the single step with no changes by itself with the different ending direction
	{
		ParkourGenerationInfo info;
		ParkourStep step;
		step.Blocks.emplace_back(std::make_pair(Voxel::NamedBlock{ "wood", Voxel::CubeData{ quat4::identity() } }, Vector::inty3(0, 0, 0)));
		step.Exits.emplace_back(LookingPoint{ inty3(), Voxel::BlockFace::Right });
		
		info.UsableSteps.push_back(step);
		info.NumGeneratedSteps = 1;
		
		const auto parkour = GenerateParkour(info);

		EXPECT_EQ(parkour.Blocks.size(), 1);
		EXPECT_EQ(parkour.Blocks.count(inty3()), 1);
		auto it = parkour.Blocks.find(inty3());
		EXPECT_TRUE(it != parkour.Blocks.end());
		EXPECT_EQ(it->second.Data.Rotation, quat4::identity());
		EXPECT_EQ(parkour.EndPosition, inty3(1, 0, 0));
		EXPECT_EQ(parkour.EndDirection, Voxel::BlockFace::Right);
	}


	// Test 1 simple step generated twice
	// Should output the single step by itself
	{
		ParkourGenerationInfo info;
		ParkourStep step;
		step.Blocks.emplace_back(std::make_pair(Voxel::NamedBlock{ "wood", Voxel::CubeData{ quat4::identity()}}, Vector::inty3(0, 0, 0)));
		step.Exits.emplace_back(LookingPoint{ inty3(), ParkourForwardDirection });

		info.UsableSteps.push_back(step);
		info.NumGeneratedSteps = 2;

		const auto parkour = GenerateParkour(info);

		EXPECT_EQ(parkour.Blocks.size(), 2);
		EXPECT_EQ(parkour.Blocks.count(inty3(0, 0, 0)), 1);
		EXPECT_EQ(parkour.Blocks.count(inty3(0, 0, -1)), 1);
		auto it = parkour.Blocks.find(inty3(0, 0, 0));
		EXPECT_TRUE(it != parkour.Blocks.end());
		EXPECT_EQ(it->second.Data.Rotation, quat4::identity());
		it = parkour.Blocks.find(inty3(0, 0, -1));
		EXPECT_TRUE(it != parkour.Blocks.end());
		EXPECT_EQ(it->second.Data.Rotation, quat4::identity());
		EXPECT_EQ(parkour.EndPosition, inty3(0, 0, -2));
		EXPECT_EQ(parkour.EndDirection, Voxel::BlockFace::Forward);
	}

	// Test 2 simple steps with a non-forward outlook direction
	{
		ParkourGenerationInfo info;
		ParkourStep step;
		step.Blocks.emplace_back(std::make_pair(Voxel::NamedBlock{ "wood", Voxel::CubeData{ quat4() } }, Vector::inty3(0, 0, 0)));
		step.Exits.emplace_back(LookingPoint{ inty3(), Voxel::BlockFace::POS_X});
		
		info.UsableSteps.push_back(step);
		info.NumGeneratedSteps = 2;
		
		const auto parkour = GenerateParkour(info);

		EXPECT_EQ(parkour.Blocks.size(), 2);
		EXPECT_EQ(parkour.Blocks.count(inty3(0, 0, 0)), 1);
		EXPECT_EQ(parkour.Blocks.count(inty3(1, 0, 0)), 1);
		auto it = parkour.Blocks.find(inty3(0, 0, 0));
		EXPECT_TRUE(it != parkour.Blocks.end());
		EXPECT_EQ(it->second.Data.Rotation, quat4::identity());
		it = parkour.Blocks.find(inty3(1, 0, 0));
		EXPECT_TRUE(it != parkour.Blocks.end());
		EXPECT_TRUE(it->second.Data.Rotation.approximately_equal(Voxel::GetWhatRotates(ParkourForwardDirection, Voxel::BlockFace::POS_X)));
		EXPECT_EQ(parkour.EndPosition, inty3(1, 0, 1));
		EXPECT_EQ(parkour.EndDirection, Voxel::BlockFace::POS_Z);
	}

	// Test 1 simple step with a forward direction generated thrice
	{
		ParkourGenerationInfo info;
		ParkourStep step;
		step.Blocks.emplace_back(std::make_pair(Voxel::NamedBlock{ "wood", Voxel::CubeData{ quat4() } }, Vector::inty3(0, 0, 0)));
		step.Exits.emplace_back(LookingPoint{ inty3(), ParkourForwardDirection });

		info.UsableSteps.push_back(step);
		info.NumGeneratedSteps = 3;

		const auto parkour = GenerateParkour(info);

		EXPECT_EQ(parkour.Blocks.size(), 3);
		EXPECT_EQ(parkour.Blocks.count(inty3(0, 0, 0)), 1);
		EXPECT_EQ(parkour.Blocks.count(inty3(0, 0, -1)), 1);
		EXPECT_EQ(parkour.Blocks.count(inty3(0, 0, -2)), 1);
		EXPECT_EQ(parkour.EndPosition, inty3(0, 0, -3));
		EXPECT_EQ(parkour.EndDirection, ParkourForwardDirection);
	}
	

	// Test 1 simple step with a non-forward direction generated thrice
	{
		ParkourGenerationInfo info;
		ParkourStep step;
		step.Blocks.emplace_back(std::make_pair(Voxel::NamedBlock{ "wood", Voxel::CubeData{ quat4() } }, Vector::inty3(0, 0, 0)));
		step.Exits.emplace_back(LookingPoint{ inty3(), Voxel::BlockFace::NEG_X });

		info.UsableSteps.push_back(step);
		info.NumGeneratedSteps = 3;

		const auto parkour = GenerateParkour(info);

		EXPECT_EQ(parkour.Blocks.size(), 3);
		EXPECT_EQ(parkour.Blocks.count(inty3(0, 0, 0)), 1);
		EXPECT_EQ(parkour.Blocks.count(inty3(-1, 0, 0)), 1);
		EXPECT_EQ(parkour.Blocks.count(inty3(-1, 0, 1)), 1);
		
		auto it = parkour.Blocks.find(inty3(0, 0, 0));
		EXPECT_TRUE(it != parkour.Blocks.end());
		EXPECT_EQ(it->second.Data.Rotation, quat4::identity());
		
		it = parkour.Blocks.find(inty3(-1, 0, 0));
		EXPECT_TRUE(it != parkour.Blocks.end());
		EXPECT_TRUE(it->second.Data.Rotation.approximately_equal(Voxel::GetWhatRotates(ParkourForwardDirection, Voxel::BlockFace::NEG_X)));
		
		it = parkour.Blocks.find(inty3(-1, 0, 1));
		EXPECT_TRUE(it != parkour.Blocks.end());
		EXPECT_TRUE(it->second.Data.Rotation.approximately_equal(Voxel::GetWhatRotates(ParkourForwardDirection, Voxel::BlockFace::NEG_X) * Voxel::GetWhatRotates(ParkourForwardDirection, Voxel::BlockFace::NEG_X)));
		
		EXPECT_EQ(parkour.EndPosition, inty3(0, 0, 1));
		auto neg_x_rot = Voxel::GetWhatRotates(ParkourForwardDirection, Voxel::BlockFace::NEG_X);
		EXPECT_EQ(parkour.EndDirection, Voxel::RotateFace(ParkourForwardDirection, neg_x_rot * neg_x_rot * neg_x_rot));
	}

	// Test 1 longer step with a forward direction generated twice
	{
		ParkourGenerationInfo info;
		ParkourStep step;
		step.Blocks.emplace_back(std::make_pair(Voxel::NamedBlock{ "wood", Voxel::CubeData{ quat4() } }, Vector::inty3(0, 0, 0)));
		step.Blocks.emplace_back(std::make_pair(Voxel::NamedBlock{ "wood", Voxel::CubeData{ quat4() } }, Vector::inty3(0, 0, -1)));
		step.Exits.emplace_back(LookingPoint{ inty3(0, 0, -1), ParkourForwardDirection });

		info.UsableSteps.push_back(step);
		info.NumGeneratedSteps = 2;
		
		const auto parkour = GenerateParkour(info);

		EXPECT_EQ(parkour.Blocks.size(), 4);
		EXPECT_EQ(parkour.Blocks.count(inty3(0, 0, 0)), 1);
		EXPECT_EQ(parkour.Blocks.count(inty3(0, 0, -1)), 1);
		EXPECT_EQ(parkour.Blocks.count(inty3(0, 0, -2)), 1);
		EXPECT_EQ(parkour.Blocks.count(inty3(0, 0, -3)), 1);
		EXPECT_TRUE(std::all_of(parkour.Blocks.begin(), parkour.Blocks.end(), [](const std::pair<const inty3, Voxel::NamedBlock>& block) { return block.second.Data.Rotation == quat4::identity(); }));
		EXPECT_EQ(parkour.EndPosition, inty3(0, 0, -4));
		EXPECT_EQ(parkour.EndDirection, ParkourForwardDirection);
	}

	// Test 1 simple longer step with a non-forward direction generated twice 
	{
		ParkourGenerationInfo info;
		ParkourStep step;
		step.Blocks.emplace_back(std::make_pair(Voxel::NamedBlock{ "wood", Voxel::CubeData{ quat4() } }, Vector::inty3(0, 0, 0)));
		step.Blocks.emplace_back(std::make_pair(Voxel::NamedBlock{ "wood", Voxel::CubeData{ quat4() } }, Vector::inty3(0, 0, -1)));
		step.Exits.emplace_back(LookingPoint{ inty3(0, 0, -1), Voxel::BlockFace::POS_X });

		info.UsableSteps.push_back(step);
		info.NumGeneratedSteps = 2;

		const auto parkour = GenerateParkour(info);

		EXPECT_EQ(parkour.Blocks.size(), 4);
		EXPECT_EQ(parkour.Blocks.count(inty3(0, 0, 0)), 1);
		EXPECT_EQ(parkour.Blocks.count(inty3(0, 0, -1)), 1);
		EXPECT_EQ(parkour.Blocks.count(inty3(1, 0, -1)), 1);
		EXPECT_EQ(parkour.Blocks.count(inty3(2, 0, -1)), 1);
		auto it = parkour.Blocks.find(inty3(0, 0, 0));
		EXPECT_NE(it, parkour.Blocks.end());
		EXPECT_EQ(it->second.Data.Rotation, quat4::identity());
		it = parkour.Blocks.find(inty3(0, 0, -1));
		EXPECT_NE(it, parkour.Blocks.end());
		EXPECT_EQ(it->second.Data.Rotation, quat4::identity());
		it = parkour.Blocks.find(inty3(1, 0, -1));
		EXPECT_NE(it, parkour.Blocks.end());
		EXPECT_TRUE(it->second.Data.Rotation.approximately_equal(Voxel::GetWhatRotates(ParkourForwardDirection, Voxel::BlockFace::POS_X)));
		it = parkour.Blocks.find(inty3(2, 0, -1));
		EXPECT_NE(it, parkour.Blocks.end());
		EXPECT_TRUE(it->second.Data.Rotation.approximately_equal(Voxel::GetWhatRotates(ParkourForwardDirection, Voxel::BlockFace::POS_X)));		
		EXPECT_EQ(parkour.EndPosition, inty3(2, 0, 0));
		EXPECT_EQ(parkour.EndDirection, Voxel::RotateFace(ParkourForwardDirection, Voxel::GetWhatRotates(ParkourForwardDirection, Voxel::BlockFace::POS_X) * Voxel::GetWhatRotates(ParkourForwardDirection, Voxel::BlockFace::POS_X)));
	}
}

#include <gmock/gmock-matchers.h>

TEST(GenerateParkourTests, DistanceTests)
{
	using namespace Voxel;
	using namespace Vector;
	using namespace Parkour;
	using ::testing::Eq;
	using ::testing::Contains;
	using ::testing::AnyOf;
	using ::testing::Pair;
	using ::testing::_;
	
	// Test empty output with a single distance
	{
		ParkourGenerationInfo info;
		info.WeightedDistances.emplace_back(std::make_pair(1.f, 1));
		
		const auto parkour = GenerateParkour(info);
		
		EXPECT_EQ(parkour.Blocks.size(), 0);
		EXPECT_EQ(parkour.EndPosition, inty3(0, 0, 0));
		EXPECT_EQ(parkour.EndDirection, ParkourForwardDirection);
	}

	// Test a singular step with a single distance
	{
		ParkourGenerationInfo info;
		ParkourStep step;
		step.Blocks.emplace_back(std::make_pair(NamedBlock{ "wood", CubeData{ quat4() } }, inty3(0, 0, 0)));
		step.Exits.emplace_back(LookingPoint{ inty3(0, 0, 0), ParkourForwardDirection });
		info.UsableSteps.push_back(step);
		info.WeightedDistances.emplace_back(std::make_pair(1.f, 1));
		info.NumGeneratedSteps = 1;

		const auto parkour = GenerateParkour(info);

		EXPECT_EQ(parkour.Blocks.size(), 1);
		EXPECT_EQ(parkour.Blocks.count(inty3(0, 0, 0)), 1);
		EXPECT_EQ(parkour.EndPosition, inty3(0, 0, -1));
		EXPECT_EQ(parkour.EndDirection, ParkourForwardDirection);
	}

	// Test a singular step with a single distance generated twice
	{
		ParkourGenerationInfo info;
		ParkourStep step;
		step.Blocks.emplace_back(std::make_pair(NamedBlock{ "wood", CubeData{ quat4() } }, inty3(0, 0, 0)));
		step.Exits.emplace_back(LookingPoint{ inty3(0, 0, 0), ParkourForwardDirection });
		info.UsableSteps.push_back(step);
		info.WeightedDistances.emplace_back(std::make_pair(1.f, 1));
		info.NumGeneratedSteps = 2;

		const auto parkour = GenerateParkour(info);
		
		EXPECT_EQ(parkour.Blocks.size(), 2);
		EXPECT_EQ(parkour.Blocks.count(inty3(0, 0, 0)), 1);
		EXPECT_EQ(parkour.Blocks.count(inty3(0, 0, -2)), 1);
		
		EXPECT_EQ(parkour.EndPosition, inty3(0, 0, -3));
		EXPECT_EQ(parkour.EndDirection, ParkourForwardDirection);
	}

	// Test a singular step with 2 distances generated twice
	{
		ParkourGenerationInfo info;
		ParkourStep step;
		step.Blocks.emplace_back(std::make_pair(NamedBlock{ "wood", CubeData{ quat4() } }, inty3(0, 0, 0)));
		step.Exits.emplace_back(LookingPoint{ inty3(0, 0, 0), ParkourForwardDirection });
		info.UsableSteps.push_back(step);
		info.WeightedDistances.emplace_back(std::make_pair(0.5f, 1));
		info.WeightedDistances.emplace_back(std::make_pair(0.5f, 2));
		info.NumGeneratedSteps = 2;

		const auto parkour = GenerateParkour(info);
		
		EXPECT_EQ(parkour.Blocks.size(), 2);
		EXPECT_EQ(parkour.Blocks.count(inty3(0, 0, 0)), 1);
		EXPECT_THAT(parkour.Blocks, Contains(AnyOf(Pair(inty3(0, 0, -2), _), Pair(inty3(0, 0, -3), _))));
		
		EXPECT_THAT(parkour.EndPosition, AnyOf(inty3(0, 0, -3), inty3(0, 0, -4)));
		EXPECT_EQ(parkour.EndDirection, ParkourForwardDirection);
	}

	// Test a singular step with 3 distances generated twice
	{
		ParkourGenerationInfo info;
		ParkourStep step;
		step.Blocks.emplace_back(std::make_pair(NamedBlock{ "wood", CubeData{ quat4() } }, inty3(0, 0, 0)));
		step.Exits.emplace_back(LookingPoint{ inty3(0, 0, 0), ParkourForwardDirection });
		info.UsableSteps.push_back(step);
		info.WeightedDistances.emplace_back(std::make_pair(0.333f, 1));
		info.WeightedDistances.emplace_back(std::make_pair(0.333f, 2));
		info.WeightedDistances.emplace_back(std::make_pair(0.333f, 3));
		info.NumGeneratedSteps = 2;

		const auto parkour = GenerateParkour(info);
		
		EXPECT_EQ(parkour.Blocks.size(), 2);
		EXPECT_EQ(parkour.Blocks.count(inty3(0, 0, 0)), 1);
		EXPECT_THAT(parkour.Blocks, Contains(AnyOf(Pair(inty3(0, 0, -2), _), Pair(inty3(0, 0, -3), _), Pair(inty3(0, 0, -4), _))));
		
		EXPECT_THAT(parkour.EndPosition, AnyOf(inty3(0, 0, -3), inty3(0, 0, -4), inty3(0, 0, -5)));
		EXPECT_EQ(parkour.EndDirection, ParkourForwardDirection);
	}

	// Test a singular step with 3 distances generated 5 times
	{
		ParkourGenerationInfo info;
		ParkourStep step;
		step.Blocks.emplace_back(std::make_pair(NamedBlock{ "wood", CubeData{ quat4() } }, inty3(0, 0, 0)));
		step.Exits.emplace_back(LookingPoint{ inty3(0, 0, 0), ParkourForwardDirection });
		info.UsableSteps.push_back(step);
		info.WeightedDistances.emplace_back(std::make_pair(0.5f, 1));
		info.WeightedDistances.emplace_back(std::make_pair(0.333f, 2));
		info.WeightedDistances.emplace_back(std::make_pair(0.333f, 3));
		info.NumGeneratedSteps = 5;

		const auto parkour = GenerateParkour(info);

		EXPECT_EQ(parkour.Blocks.size(), 5);
		EXPECT_EQ(parkour.Blocks.count(inty3(0, 0, 0)), 1);
		inty3 pos = inty3(0, 0, -2);
		for (int step = 0; step < 4; ++step)
		{
			EXPECT_THAT(parkour.Blocks, Contains(AnyOf(Pair(pos, _), Pair(pos - inty3(0, 0, 1), _), Pair(pos - inty3(0, 0, 2), _))));
			for (int i = 0; i < 4; ++i)
			{
				if (i == 4)
				{
					ADD_FAILURE();
					break;
				}

				auto it = parkour.Blocks.find(pos - inty3(0, 0, i));
				if (it != parkour.Blocks.end())
				{
					pos.z -= i + 2;
					break;
				}
			}
		}
		
		EXPECT_EQ(parkour.EndDirection, ParkourForwardDirection);
	}
}

TEST(GenerateParkourTests, ComplexParkourTests)
{
	using namespace Voxel;
	using namespace Vector;
	using namespace Parkour;

	using ::testing::UnorderedElementsAre;
	using ::testing::Pair;
	using ::testing::_;
	
	// Test a straight 4-piece long step generated 5 times
	{
		ParkourGenerationInfo info;
		ParkourStep step;

		step.Blocks.emplace_back(std::make_pair(NamedBlock{ "wood", CubeData{ quat4() } }, Vector::inty3(0, 0, 0)));
		step.Blocks.emplace_back(std::make_pair(NamedBlock{ "wood", CubeData{ quat4() } }, Vector::inty3(0, 0, -1)));
		step.Blocks.emplace_back(std::make_pair(NamedBlock{ "wood", CubeData{ quat4() } }, Vector::inty3(0, 0, -2)));
		step.Blocks.emplace_back(std::make_pair(NamedBlock{ "wood", CubeData{ quat4() } }, Vector::inty3(0, 0, -3)));
		step.Exits.emplace_back(LookingPoint{ inty3(0, 0, -3), ParkourForwardDirection });
		
		info.UsableSteps.push_back(step);
		
		info.NumGeneratedSteps = 5;
		
		const auto parkour = GenerateParkour(info);
		
		EXPECT_EQ(parkour.Blocks.size(), 20);
		for (int i = 0; i < 20; ++i)
		{
			EXPECT_EQ(parkour.Blocks.count(inty3(0, 0, -i)), 1);
		}
		EXPECT_TRUE(std::all_of(parkour.Blocks.begin(), parkour.Blocks.end(), [](const std::pair<const inty3, Voxel::NamedBlock>& block) { return block.second.Data.Rotation == quat4::identity(); }));
		
		EXPECT_EQ(parkour.EndPosition, inty3(0, 0, -20));
		EXPECT_EQ(parkour.EndDirection, ParkourForwardDirection);
	}
	
	// Test a single step with a non-forward exit generated 3 times
	// Step:
	//  XX
	//   X
	//   X
	{
		ParkourGenerationInfo info;
		ParkourStep step;

		step.Blocks.emplace_back(std::make_pair(NamedBlock{ "wood", CubeData{ quat4() } }, Vector::inty3(0, 0, 0)));
		step.Blocks.emplace_back(std::make_pair(NamedBlock{ "wood", CubeData{ quat4() } }, Vector::inty3(0, 0, -1)));
		step.Blocks.emplace_back(std::make_pair(NamedBlock{ "wood", CubeData{ quat4() } }, Vector::inty3(0, 0, -2)));
		step.Blocks.emplace_back(std::make_pair(NamedBlock{ "wood", CubeData{quat4()}}, Vector::inty3(1, 0, -2)));
		step.Exits.emplace_back(LookingPoint{ inty3(1, 0, -2), ParkourRightDirection });

		info.UsableSteps.push_back(step);

		info.NumGeneratedSteps = 3;

		const auto parkour = GenerateParkour(info);

		EXPECT_EQ(parkour.Blocks.size(), 12);
		
		EXPECT_THAT(parkour.Blocks, UnorderedElementsAre(
			Pair(inty3(0, 0, 0), _),
			Pair(inty3(0, 0, -1), _),
			Pair(inty3(0, 0, -2), _),
			Pair(inty3(1, 0, -2), _),

			Pair(inty3(2, 0, -2), _),
			Pair(inty3(3, 0, -2), _),
			Pair(inty3(4, 0, -2), _),
			Pair(inty3(4, 0, -1), _),

			Pair(inty3(4, 0, 0), _),
			Pair(inty3(4, 0, 1), _),
			Pair(inty3(4, 0, 2), _),
			Pair(inty3(3, 0, 2), _)
		));

		EXPECT_EQ(parkour.EndPosition, inty3(2, 0, 2));
		EXPECT_EQ(parkour.EndDirection, ParkourLeftDirection);
	}
}

TEST(ParkourTests, ConversionFuncTests)
{
	using namespace Voxel;
	using namespace Vector;
	using namespace Parkour;

	// Test inty3ToBlockCoord and BlockCoordTointy3
	
	{
		const auto inty3 = Vector::inty3(1, 2, 3);
		const auto blockCoord = inty3ToBlockCoord(inty3);
		EXPECT_EQ(blockCoord, inty3ToBlockCoord(Vector::inty3(1, 2, 3)));
	}

	{
		const auto inty3 = Vector::inty3(Chunk_Size * 3, Chunk_Height * 5, Chunk_Size * 7);
		const auto blockCoord = inty3ToBlockCoord(inty3);
		EXPECT_EQ(blockCoord, (Voxel::BlockCoord{ Voxel::ChunkCoord{ 3, 5, 7 }, 0, 0, 0 }));
	}

	{
		const auto inty3 = Vector::inty3(Chunk_Size * 3 + 3, Chunk_Height * 5 + 4, Chunk_Size * 7 + 2);
		const auto blockCoord = inty3ToBlockCoord(inty3);
		EXPECT_EQ(blockCoord, (Voxel::BlockCoord{ Voxel::ChunkCoord{ 3, 5, 7 }, 3, 4, 2 }));
	}

	// Negative values

	{
		const auto pos = Vector::inty3(Chunk_Size * -2 - 4, Chunk_Height * -1, Chunk_Size * -3 - 2);
		const auto blockCoord = inty3ToBlockCoord(pos);
		EXPECT_EQ(blockCoord, (Voxel::BlockCoord{ Voxel::ChunkCoord{ -3, -1, -4 }, Chunk_Size - 4, 0, Chunk_Size - 2 }));
	}

	{
		const auto pos = Vector::inty3(Chunk_Size * -2 - 4, Chunk_Height * -1 - 1, Chunk_Size * -3 - 2);
		const auto blockCoord = inty3ToBlockCoord(pos);
		EXPECT_EQ(blockCoord, (Voxel::BlockCoord{ Voxel::ChunkCoord{ -3, -2, -4 }, Chunk_Size - 4, Chunk_Height - 1, Chunk_Size - 2 }));
	}


	// Now BlockCoordTointy3

	{
		const auto blockCoord = Voxel::BlockCoord{ Voxel::ChunkCoord{ 3, 5, 7 }, 0, 0, 0 };
		const auto inty3 = BlockCoordTointy3(blockCoord);
		EXPECT_EQ(inty3, Vector::inty3(Chunk_Size * 3, Chunk_Height * 5, Chunk_Size * 7));
	}

	{
		const auto blockCoord = Voxel::BlockCoord{ Voxel::ChunkCoord{ 3, 5, 7 }, 3, 4, 2 };
		const auto inty3 = BlockCoordTointy3(blockCoord);
		EXPECT_EQ(inty3, Vector::inty3(Chunk_Size * 3 + 3, Chunk_Height * 5 + 4, Chunk_Size * 7 + 2));
	}

	// Negative values

	{
		const auto blockCoord = Voxel::BlockCoord{ Voxel::ChunkCoord{ -3, -1, -4 }, Chunk_Size - 4, 0, Chunk_Size - 2 };
		const auto inty3 = BlockCoordTointy3(blockCoord);
		EXPECT_EQ(inty3, Vector::inty3(Chunk_Size * -2 - 4, Chunk_Height * -1, Chunk_Size * -3 - 2));
	}

	{
		const auto blockCoord = Voxel::BlockCoord{ Voxel::ChunkCoord{ -3, -2, -4 }, Chunk_Size - 4, Chunk_Height - 1, Chunk_Size - 2 };
		const auto inty3 = BlockCoordTointy3(blockCoord);
		EXPECT_EQ(inty3, Vector::inty3(Chunk_Size * -2 - 4, Chunk_Height * -1 - 1, Chunk_Size * -3 - 2));
	}
}

#endif

