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
				std::vector<std::pair<Voxel::NamedBlock, Vector::inty3>>{
					std::make_pair(Voxel::NamedBlock{ "wood", Voxel::CubeData{ quat4::identity() } }, Vector::inty3{ 0, 0, 0 }),
					std::make_pair(Voxel::NamedBlock{ "wood", Voxel::CubeData{ quat4::identity() } }, Vector::inty3{ 0, 0, -1 }),
					std::make_pair(Voxel::NamedBlock{ "wood", Voxel::CubeData{ quat4::identity() } }, Vector::inty3{ 0, 0, -2 }),
					std::make_pair(Voxel::NamedBlock{ "wood", Voxel::CubeData{ quat4::identity() } }, Vector::inty3{ -1, 0, -2 }),
					std::make_pair(Voxel::NamedBlock{ "lamp-base", Voxel::CubeData{ quat4::identity() } }, Vector::inty3{ -1, 1, -2 }),
					std::make_pair(Voxel::NamedBlock{ "lamp-pole", Voxel::CubeData{ quat4::identity() } }, Vector::inty3{ -1, 2, -2 }),
					std::make_pair(Voxel::NamedBlock{ "lamp-pole", Voxel::CubeData{ quat4::identity() } }, Vector::inty3{ -1, 3, -2 }),
					std::make_pair(Voxel::NamedBlock{ "lamp-bend", Voxel::CubeData{ quat4(btQuaternion::btQuaternion(-90.f * Math::DegToRadF, 0.f, 0.f)) } }, Vector::inty3{ -1, 4, -2 }),
					std::make_pair(Voxel::NamedBlock{ "lamp-light", Voxel::CubeData{ quat4(btQuaternion::btQuaternion(-90.f * Math::DegToRadF, 0.f, 0.f)) } }, Vector::inty3{ 0, 4, -2 }),
					std::make_pair(Voxel::NamedBlock{ "wood", Voxel::CubeData{ quat4::identity() } }, Vector::inty3{ 0, 0, -3 }),
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
			std::vector<std::pair<Voxel::NamedBlock, Vector::inty3>> {
				std::make_pair(Voxel::NamedBlock{ "wood", Voxel::CubeData{ quat4::identity() } }, Vector::inty3{ 0, 0, 0 }),
				std::make_pair(Voxel::NamedBlock{ "wood", Voxel::CubeData{ quat4::identity() } }, Vector::inty3{ 0, 0, -1 }),
				std::make_pair(Voxel::NamedBlock{ "wood", Voxel::CubeData{ quat4::identity() } }, Vector::inty3{ 1, 0, -2 }),
				std::make_pair(Voxel::NamedBlock{ "wood", Voxel::CubeData{ quat4::identity() } }, Vector::inty3{ 1, 0, -3 }),
				std::make_pair(Voxel::NamedBlock{ "wood", Voxel::CubeData{ quat4::identity() } }, Vector::inty3{ -1, 0, -2 }),
				std::make_pair(Voxel::NamedBlock{ "wood", Voxel::CubeData{ quat4::identity() } }, Vector::inty3{ -1, 0, -3 }),

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
			std::vector<std::pair<Voxel::NamedBlock, Vector::inty3>>{
				std::make_pair(Voxel::NamedBlock{ "wood", Voxel::CubeData{ quat4::identity() } }, Vector::inty3{ 0, 0, 0 }),
				std::make_pair(Voxel::NamedBlock{ "wood", Voxel::CubeData{ quat4::identity() } }, Vector::inty3{ -2, 0, -1 }),
				std::make_pair(Voxel::NamedBlock{ "wood", Voxel::CubeData{ quat4::identity() } }, Vector::inty3{ +2, 0, -1 }),
				std::make_pair(Voxel::NamedBlock{ "wood", Voxel::CubeData{ quat4::identity() } }, Vector::inty3{ 0, 0, -2 }),
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
			std::vector<std::pair<Voxel::NamedBlock, Vector::inty3>>{
				std::make_pair(Voxel::NamedBlock{ "wood", Voxel::CubeData{ quat4::identity() } }, Vector::inty3{ 0, 0, 0 }),
				std::make_pair(Voxel::NamedBlock{ "wood", Voxel::CubeData{ quat4::identity() } }, Vector::inty3{ 0, 0, -1 }),
				std::make_pair(Voxel::NamedBlock{ "wood", Voxel::CubeData{ quat4::identity() } }, Vector::inty3{ 0, 1, -3 }),
				std::make_pair(Voxel::NamedBlock{ "wood", Voxel::CubeData{ quat4::identity() } }, Vector::inty3{ 0, 1, -4 }),
				std::make_pair(Voxel::NamedBlock{ "wood", Voxel::CubeData{ quat4::identity() } }, Vector::inty3{ 0, 2, -6 }),
				std::make_pair(Voxel::NamedBlock{ "wood", Voxel::CubeData{ quat4::identity() } }, Vector::inty3{ 0, 2, -7 }),
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
			std::vector<std::pair<Voxel::NamedBlock, Vector::inty3>>{
				std::make_pair(Voxel::NamedBlock{ "wood", Voxel::CubeData{ quat4::identity() } }, Vector::inty3{ 0, 0, 0 }),
				std::make_pair(Voxel::NamedBlock{ "wood", Voxel::CubeData{ quat4::identity() } }, Vector::inty3{ 1, 0, -1 }),
				std::make_pair(Voxel::NamedBlock{ "wood", Voxel::CubeData{ quat4::identity() } }, Vector::inty3{ 2, 0, -2 }),
				std::make_pair(Voxel::NamedBlock{ "wood", Voxel::CubeData{ quat4::identity() } }, Vector::inty3{ 3, 0, -2 }),
				std::make_pair(Voxel::NamedBlock{ "wood", Voxel::CubeData{ quat4::identity() } }, Vector::inty3{ 4, 0, -3 }),
				std::make_pair(Voxel::NamedBlock{ "wood", Voxel::CubeData{ quat4::identity() } }, Vector::inty3{ 5, 0, -4 }),
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
			std::vector<std::pair<Voxel::NamedBlock, Vector::inty3>>{
				std::make_pair(Voxel::NamedBlock{ "wood", Voxel::CubeData{ quat4::identity() } }, Vector::inty3{ -0, 0, 0 }),
				std::make_pair(Voxel::NamedBlock{ "wood", Voxel::CubeData{ quat4::identity() } }, Vector::inty3{ -1, 0, -1 }),
				std::make_pair(Voxel::NamedBlock{ "wood", Voxel::CubeData{ quat4::identity() } }, Vector::inty3{ -2, 0, -2 }),
				std::make_pair(Voxel::NamedBlock{ "wood", Voxel::CubeData{ quat4::identity() } }, Vector::inty3{ -3, 0, -2 }),
				std::make_pair(Voxel::NamedBlock{ "wood", Voxel::CubeData{ quat4::identity() } }, Vector::inty3{ -4, 0, -3 }),
				std::make_pair(Voxel::NamedBlock{ "wood", Voxel::CubeData{ quat4::identity() } }, Vector::inty3{ -5, 0, -4 }),
			},
			std::vector<LookingPoint>{
				LookingPoint{ Vector::inty3(-5, 0, -4), ParkourForwardDirection }
			}
		};

		// Topdown:
		// #O
		const ParkourStep SingleBlockStepDecorLeft = ParkourStep
		{
			std::vector<std::pair<Voxel::NamedBlock, Vector::inty3>>{
				std::make_pair(Voxel::NamedBlock{ "wood", Voxel::CubeData{ quat4::identity() } }, Vector::inty3{ 0, 0, 0 }),
				std::make_pair(Voxel::NamedBlock{ "wood", Voxel::CubeData{ quat4::identity() } }, Vector::inty3{ -1, 0, 0 }),
				std::make_pair(Voxel::NamedBlock{ "potted-plant", Voxel::CubeData{ quat4::identity() } }, Vector::inty3{ -1, 1, 0 }),
			},
			std::vector<LookingPoint>{
				LookingPoint{ Vector::inty3(0, 0, 0), ParkourForwardDirection },
				LookingPoint{ Vector::inty3(0, 0, 0), ParkourRightDirection }
			}
		};

		// Topdown:
		// O#
		const ParkourStep SingleBlockStepDecorRight = ParkourStep
		{
			std::vector<std::pair<Voxel::NamedBlock, Vector::inty3>>{
				std::make_pair(Voxel::NamedBlock{ "wood", Voxel::CubeData{ quat4::identity() } }, Vector::inty3{ 0, 0, 0 }),
				std::make_pair(Voxel::NamedBlock{ "wood", Voxel::CubeData{ quat4::identity() } }, Vector::inty3{ 1, 0, 0 }),
				std::make_pair(Voxel::NamedBlock{ "potted-plant", Voxel::CubeData{ quat4::identity() } }, Vector::inty3{ 1, 1, 0 }),
			},
			std::vector<LookingPoint>{
				LookingPoint{ Vector::inty3(0, 0, 0), ParkourForwardDirection },
				LookingPoint{ Vector::inty3(0, 0, 0), ParkourLeftDirection }
			}
		};

		// Topdown:
		//    ^
		// < XOX >
		const ParkourStep TripleSidewaysStep = ParkourStep
		{
			std::vector<std::pair<Voxel::NamedBlock, Vector::inty3>>{
				std::make_pair(Voxel::NamedBlock{ "wood", Voxel::CubeData{ quat4::identity() } }, Vector::inty3{ 0, 0, 0 }),
				std::make_pair(Voxel::NamedBlock{ "wood", Voxel::CubeData{ quat4::identity() } }, Vector::inty3{ -1, 0, 0 }),
				std::make_pair(Voxel::NamedBlock{ "wood", Voxel::CubeData{ quat4::identity() } }, Vector::inty3{ +1, 0, 0 })
			},
			std::vector<LookingPoint>{
				LookingPoint{ Vector::inty3(0, 0, 0), ParkourForwardDirection },
				LookingPoint{ Vector::inty3(+1, 0, 0), ParkourRightDirection },
				LookingPoint{ Vector::inty3(-1, 0, 0), ParkourLeftDirection }
			}
		};

		// Topdown:
		//   ^
		//  XOX >
		// Sideview:
		//  #
		//  #
		//  XOX
		const ParkourStep TripleSidewaysStepWithSign = ParkourStep
		{
			std::vector<std::pair<Voxel::NamedBlock, Vector::inty3>>{
				std::make_pair(Voxel::NamedBlock{ "wood", Voxel::CubeData{ quat4::identity()}}, Vector::inty3{0, 0, 0}),
				std::make_pair(Voxel::NamedBlock{ "wood", Voxel::CubeData{ quat4::identity() } }, Vector::inty3{ -1, 0, 0 }),
				std::make_pair(Voxel::NamedBlock{ "sign-base", Voxel::CubeData{ Voxel::GetWhatRotates(Voxel::BlockFace::FORWARD, Voxel::BlockFace::RIGHT) } }, Vector::inty3{ -1, 1, 0 }),
				std::make_pair(Voxel::NamedBlock{ "sign-board", Voxel::CubeData{ Voxel::GetWhatRotates(Voxel::BlockFace::FORWARD, Voxel::BlockFace::RIGHT) } }, Vector::inty3{ -1, 2, 0 }),
				std::make_pair(Voxel::NamedBlock{ "wood", Voxel::CubeData{ quat4::identity() } }, Vector::inty3{ +1, 0, 0 })
			},
			std::vector<LookingPoint>{
				LookingPoint{ Vector::inty3(0, 0, 0), ParkourForwardDirection },
				LookingPoint{ Vector::inty3(+1, 0, 0), ParkourRightDirection }
			}
		};
		

		// Topdown:
		// ##O##
		const ParkourStep DecoratedSingleStep = ParkourStep
		{
			std::vector<std::pair<Voxel::NamedBlock, Vector::inty3>>{
				std::make_pair(Voxel::NamedBlock{ "wood", Voxel::CubeData{ quat4::identity() } }, Vector::inty3(0, 0, 0)),
				std::make_pair(Voxel::NamedBlock{ "wood", Voxel::CubeData{ quat4::identity() } }, Vector::inty3(-1, 0, 0)),
				std::make_pair(Voxel::NamedBlock{ "lamp-base", Voxel::CubeData{ quat4::identity() } }, Vector::inty3(-1, 1, 0)),
				std::make_pair(Voxel::NamedBlock{ "lamp-pole", Voxel::CubeData{ quat4::identity() } }, Vector::inty3(-1, 2, 0)),
				std::make_pair(Voxel::NamedBlock{ "lamp-bend", Voxel::CubeData{ quat4::identity() } }, Vector::inty3(-1, 3, 0)),
				std::make_pair(Voxel::NamedBlock{ "lamp-light", Voxel::CubeData{ quat4::identity() } }, Vector::inty3(0, 3, 0)),
				std::make_pair(Voxel::NamedBlock{ "wood", Voxel::CubeData{ quat4::identity() } }, Vector::inty3(-2, 0, 0)),
				std::make_pair(Voxel::NamedBlock{ "potted-hedge", Voxel::CubeData{ quat4::identity() } }, Vector::inty3(-2, 1, 0)),
				std::make_pair(Voxel::NamedBlock{ "wood", Voxel::CubeData{ quat4::identity() } }, Vector::inty3(1, 0, 0)),
				std::make_pair(Voxel::NamedBlock{ "potted-plant", Voxel::CubeData{ quat4::identity() } }, Vector::inty3(1, 1, 0)),
				std::make_pair(Voxel::NamedBlock{ "wood", Voxel::CubeData{ quat4::identity() } }, Vector::inty3(2, 0, 0)),
				std::make_pair(Voxel::NamedBlock{ "torch-base", Voxel::CubeData{ quat4::identity() } }, Vector::inty3(2, 1, 0)),
				std::make_pair(Voxel::NamedBlock{ "torch-light", Voxel::CubeData{ quat4::identity() } }, Vector::inty3(2, 2, 0)),
				std::make_pair(Voxel::NamedBlock{ "wood", Voxel::CubeData{ quat4::identity() } }, Vector::inty3(2, 0, 0)),
			},
			std::vector<LookingPoint>{
				LookingPoint{ Vector::inty3(0, 0, 0), ParkourForwardDirection }
			}
		};

		// Sideview
		// X
		// 
		// O
		const ParkourStep LampSingleStep = ParkourStep
		{
			std::vector<std::pair<Voxel::NamedBlock, Vector::inty3>>{
				std::make_pair(Voxel::NamedBlock{ "wood", Voxel::CubeData{ quat4::identity() } }, Vector::inty3(0, 0, 0)),
				std::make_pair(Voxel::NamedBlock{ "lamp-light", Voxel::CubeData{ quat4::identity() } }, Vector::inty3(0, 5, 0)),
			},
			std::vector<LookingPoint>{
				LookingPoint{ Vector::inty3(0, 0, 0), ParkourBackDirection }
			}
		};
	}

	const std::array<Parkour::ParkourLevel, 5> Levels = {
		Parkour::ParkourLevel{
			Voxel::BlockCoord{ Voxel::ChunkCoord{ 0,0,0 }, Voxel::ChunkBlockCoord{0, 2, 5 }},
			Voxel::BlockFace::Forward,
			std::vector<ParkourStep>{ Parkour::Steps::TripleSidewaysStepWithSign },
			std::vector<std::pair<float, int>>{ std::make_pair(0.5f, 1), std::make_pair(1.0f, 2), std::make_pair(0.5f, 3) },
			std::vector<std::pair<float, int>>{ std::make_pair(0.25f, 0), std::make_pair(0.75f, 1) },
			25,
			Voxel::BlockCoord{},
			Voxel::BlockFace{}
			},
		Parkour::ParkourLevel{
			Voxel::BlockCoord::Origin(),
			Voxel::BlockFace::Forward,
			std::vector<ParkourStep>{ Parkour::Steps::Long4LineStep, Parkour::Steps::SingleBlockStepDecorLeft, Parkour::Steps::Fork2_2Step, Parkour::Steps::TriExitGaps, Parkour::Steps::TripleSidewaysStep, Parkour::Steps::HolyStairs },
			std::vector<std::pair<float, int>>{ std::make_pair(0.25f, 1), std::make_pair(0.5f, 2), std::make_pair(1.5f, 3), std::make_pair(0.15f, 4) },
			std::vector<std::pair<float, int>>{ std::make_pair(0.5f, 0), std::make_pair(0.5f, 1), std::make_pair(0.2f, 2) },
			50,
			Voxel::BlockCoord{ Voxel::ChunkCoord{ 0, 0, 0 }, { 0, 2, 0 } },
			Parkour::ParkourForwardDirection
			},
		Parkour::ParkourLevel{
			Voxel::BlockCoord::Origin(),
			Voxel::BlockFace::Forward,
			std::vector<ParkourStep>{ Parkour::Steps::Long4LineStep, Parkour::Steps::SingleBlockStepDecorLeft, Parkour::Steps::Fork2_2Step, Parkour::Steps::TriExitGaps, Parkour::Steps::TripleSidewaysStep, Parkour::Steps::HolyStairs },
			std::vector<std::pair<float, int>>{ std::make_pair(0.25f, 1), std::make_pair(0.5f, 2), std::make_pair(1.5f, 3), std::make_pair(0.15f, 4) },
			std::vector<std::pair<float, int>>{ std::make_pair(0.5f, 0), std::make_pair(0.5f, 1), std::make_pair(0.2f, 2) },
			50,
			Voxel::BlockCoord{ Voxel::ChunkCoord{ 0, 0, 0 }, { 0, 2, 0 } },
			Parkour::ParkourForwardDirection
			},
	};
}