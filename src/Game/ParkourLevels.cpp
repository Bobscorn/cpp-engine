#include "ParkourLevels.h"

namespace Parkour
{
	namespace Steps
	{
		// Topdown:
		// X
		// X
		// X
		// O
		const ParkourStep Long4LineStep =
			ParkourStep{
				std::vector<std::pair<Voxel::SerialBlock, Vector::inty3>>{
					std::make_pair(Voxel::SerialBlock{ "wood", Voxel::CubeData{ quat4::identity() } }, Vector::inty3{ 0, 0, 0 }),
					std::make_pair(Voxel::SerialBlock{ "wood", Voxel::CubeData{ quat4::identity() } }, Vector::inty3{ 0, 0, -1 }),
					std::make_pair(Voxel::SerialBlock{ "wood", Voxel::CubeData{ quat4::identity() } }, Vector::inty3{ 0, 0, -2 }),
					std::make_pair(Voxel::SerialBlock{ "wood", Voxel::CubeData{ quat4::identity() } }, Vector::inty3{ 0, 0, -3 }),
				},
				std::vector<LookingPoint>{ LookingPoint{ Vector::inty3{ 0, 0, -3 }, ParkourForwardDirection } }
		};

		// Topdown:
		// X X
		// X X
		//  X
		//  O
		const ParkourStep Fork2_2Step = ParkourStep
		{
			std::vector<std::pair<Voxel::SerialBlock, Vector::inty3>> {
				std::make_pair(Voxel::SerialBlock{ "wood", Voxel::CubeData{ quat4::identity() } }, Vector::inty3{ 0, 0, 0 }),
				std::make_pair(Voxel::SerialBlock{ "wood", Voxel::CubeData{ quat4::identity() } }, Vector::inty3{ 0, 0, -1 }),
				std::make_pair(Voxel::SerialBlock{ "wood", Voxel::CubeData{ quat4::identity() } }, Vector::inty3{ 1, 0, -2 }),
				std::make_pair(Voxel::SerialBlock{ "wood", Voxel::CubeData{ quat4::identity() } }, Vector::inty3{ 1, 0, -3 }),
				std::make_pair(Voxel::SerialBlock{ "wood", Voxel::CubeData{ quat4::identity() } }, Vector::inty3{ -1, 0, -2 }),
				std::make_pair(Voxel::SerialBlock{ "wood", Voxel::CubeData{ quat4::identity() } }, Vector::inty3{ -1, 0, -3 }),

			},
			std::vector<LookingPoint>{
					LookingPoint{ Vector::inty3(+1, 0, -3), ParkourForwardDirection },
					LookingPoint{ Vector::inty3(-1, 0, -3), ParkourForwardDirection }
			}
		};

		// Topdown:
		//   X
		// X   X
		//   O
		const ParkourStep TriExitGaps = ParkourStep
		{
			std::vector<std::pair<Voxel::SerialBlock, Vector::inty3>>{
				std::make_pair(Voxel::SerialBlock{ "wood", Voxel::CubeData{ quat4::identity() } }, Vector::inty3{ 0, 0, 0 }),
				std::make_pair(Voxel::SerialBlock{ "wood", Voxel::CubeData{ quat4::identity() } }, Vector::inty3{ -2, 0, -1 }),
				std::make_pair(Voxel::SerialBlock{ "wood", Voxel::CubeData{ quat4::identity() } }, Vector::inty3{ +2, 0, -1 }),
				std::make_pair(Voxel::SerialBlock{ "wood", Voxel::CubeData{ quat4::identity() } }, Vector::inty3{ 0, 0, -2 }),
			},
			std::vector<LookingPoint>{
				LookingPoint{ Vector::inty3(-2, 0, -1), ParkourLeftDirection },
				LookingPoint{ Vector::inty3(+2, 0, -1), ParkourRightDirection },
				LookingPoint{ Vector::inty3(0, 0, -2), ParkourForwardDirection }
			}
		};

		// Sideview:
		//         XX
		//     XX
		// XX
		const ParkourStep HolyStairs = ParkourStep
		{
			std::vector<std::pair<Voxel::SerialBlock, Vector::inty3>>{
				std::make_pair(Voxel::SerialBlock{ "wood", Voxel::CubeData{ quat4::identity() } }, Vector::inty3{ 0, 0, 0 }),
				std::make_pair(Voxel::SerialBlock{ "wood", Voxel::CubeData{ quat4::identity() } }, Vector::inty3{ 0, 0, -1 }),
				std::make_pair(Voxel::SerialBlock{ "wood", Voxel::CubeData{ quat4::identity() } }, Vector::inty3{ 0, 1, -3 }),
				std::make_pair(Voxel::SerialBlock{ "wood", Voxel::CubeData{ quat4::identity() } }, Vector::inty3{ 0, 1, -4 }),
				std::make_pair(Voxel::SerialBlock{ "wood", Voxel::CubeData{ quat4::identity() } }, Vector::inty3{ 0, 2, -6 }),
				std::make_pair(Voxel::SerialBlock{ "wood", Voxel::CubeData{ quat4::identity() } }, Vector::inty3{ 0, 2, -7 }),
			},
			std::vector<LookingPoint>{
				LookingPoint{ Vector::inty3(0, 2, -7), ParkourForwardDirection }
			}
		};

		// Topdown:
		//      X
		//     X
		//   XX
		//  X
		// O
		const ParkourStep CurvyStraightRight = ParkourStep
		{
			std::vector<std::pair<Voxel::SerialBlock, Vector::inty3>>{
				std::make_pair(Voxel::SerialBlock{ "wood", Voxel::CubeData{ quat4::identity() } }, Vector::inty3{ 0, 0, 0 }),
				std::make_pair(Voxel::SerialBlock{ "wood", Voxel::CubeData{ quat4::identity() } }, Vector::inty3{ 1, 0, -1 }),
				std::make_pair(Voxel::SerialBlock{ "wood", Voxel::CubeData{ quat4::identity() } }, Vector::inty3{ 2, 0, -2 }),
				std::make_pair(Voxel::SerialBlock{ "wood", Voxel::CubeData{ quat4::identity() } }, Vector::inty3{ 3, 0, -2 }),
				std::make_pair(Voxel::SerialBlock{ "wood", Voxel::CubeData{ quat4::identity() } }, Vector::inty3{ 4, 0, -3 }),
				std::make_pair(Voxel::SerialBlock{ "wood", Voxel::CubeData{ quat4::identity() } }, Vector::inty3{ 5, 0, -4 }),
			},
			std::vector<LookingPoint>{
				LookingPoint{ Vector::inty3(5, 0, -4), ParkourForwardDirection }
			}
		};

		// Topdown:
		// X
		//  X
		//   XX
		//     X
		//      O
		const ParkourStep CurvyStraightLeft = ParkourStep
		{
			std::vector<std::pair<Voxel::SerialBlock, Vector::inty3>>{
				std::make_pair(Voxel::SerialBlock{ "wood", Voxel::CubeData{ quat4::identity() } }, Vector::inty3{ -0, 0, 0 }),
				std::make_pair(Voxel::SerialBlock{ "wood", Voxel::CubeData{ quat4::identity() } }, Vector::inty3{ -1, 0, -1 }),
				std::make_pair(Voxel::SerialBlock{ "wood", Voxel::CubeData{ quat4::identity() } }, Vector::inty3{ -2, 0, -2 }),
				std::make_pair(Voxel::SerialBlock{ "wood", Voxel::CubeData{ quat4::identity() } }, Vector::inty3{ -3, 0, -2 }),
				std::make_pair(Voxel::SerialBlock{ "wood", Voxel::CubeData{ quat4::identity() } }, Vector::inty3{ -4, 0, -3 }),
				std::make_pair(Voxel::SerialBlock{ "wood", Voxel::CubeData{ quat4::identity() } }, Vector::inty3{ -5, 0, -4 }),
			},
			std::vector<LookingPoint>{
				LookingPoint{ Vector::inty3(-5, 0, -4), ParkourForwardDirection }
			}
		};

		// Topdown:
		// O
		const ParkourStep SingleBlockStep = ParkourStep
		{
			std::vector<std::pair<Voxel::SerialBlock, Vector::inty3>>{
				std::make_pair(Voxel::SerialBlock{ "wood", Voxel::CubeData{ quat4::identity() } }, Vector::inty3{ 0, 0, 0 })
			},
			std::vector<LookingPoint>{
				LookingPoint{ Vector::inty3(0, 0, 0), ParkourForwardDirection },
				LookingPoint{ Vector::inty3(0, 0, 0), ParkourRightDirection },
				LookingPoint{ Vector::inty3(0, 0, 0), ParkourLeftDirection }
			}
		};

		// Topdown:
		// XOX
		const ParkourStep TripleSidewaysStep = ParkourStep
		{
			std::vector<std::pair<Voxel::SerialBlock, Vector::inty3>>{
				std::make_pair(Voxel::SerialBlock{ "wood", Voxel::CubeData{ quat4::identity() } }, Vector::inty3{ 0, 0, 0 }),
				std::make_pair(Voxel::SerialBlock{ "wood", Voxel::CubeData{ quat4::identity() } }, Vector::inty3{ -1, 0, 0 }),
				std::make_pair(Voxel::SerialBlock{ "wood", Voxel::CubeData{ quat4::identity() } }, Vector::inty3{ +1, 0, 0 })
			},
			std::vector<LookingPoint>{
				LookingPoint{ Vector::inty3(0, 0, 0), ParkourForwardDirection },
				LookingPoint{ Vector::inty3(+1, 0, 0), ParkourRightDirection },
				LookingPoint{ Vector::inty3(-1, 0, 0), ParkourLeftDirection }
			}
		};
			
	}

	const std::array<Parkour::ParkourLevel, 5> Levels = {
		Parkour::ParkourLevel{
			Voxel::BlockCoord::Origin(),
			Voxel::BlockFace::Forward,
			std::vector<ParkourStep>{ Parkour::Steps::Long4LineStep, Parkour::Steps::SingleBlockStep, Parkour::Steps::Fork2_2Step, Parkour::Steps::TriExitGaps, Parkour::Steps::TripleSidewaysStep, Parkour::Steps::HolyStairs },
			std::vector<std::pair<float, int>>{ std::make_pair(0.5f, 1), std::make_pair(1.0f, 2), std::make_pair(0.5f, 3) },
			std::vector<std::pair<float, int>>{ std::make_pair(0.5f, 0), std::make_pair(0.5f, 1) },
			30,
			Voxel::BlockCoord{ Voxel::ChunkCoord{ 0, 0, 0 }, { 0, 2, 0 } },
			Parkour::ParkourForwardDirection
			},
	};
}