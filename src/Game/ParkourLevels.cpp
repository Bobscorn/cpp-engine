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
					std::make_pair(Voxel::NamedBlock{ "lamp-bend", Voxel::CubeData{ quat4(btQuaternion(-90.f * Math::DegToRadF, 0.f, 0.f)) } }, Vector::inty3{ -1, 4, -2 }),
					std::make_pair(Voxel::NamedBlock{ "lamp-light", Voxel::CubeData{ quat4(btQuaternion(-90.f * Math::DegToRadF, 0.f, 0.f)) } }, Vector::inty3{ 0, 4, -2 }),
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
				std::make_pair(Voxel::NamedBlock{ "sign-base", Voxel::CubeData{ quat4::identity() }}, Vector::inty3{-1, 1, 0}),
				std::make_pair(Voxel::NamedBlock{ "sign-board", Voxel::CubeData{ quat4::identity() }}, Vector::inty3{-1, 2, 0}),
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
				std::make_pair(Voxel::NamedBlock{ "lamp-bend", Voxel::CubeData{ quat4(btQuaternion(-90.f * Math::DegToRadF, 0.f, 0.f)) } }, Vector::inty3(-1, 3, 0)),
				std::make_pair(Voxel::NamedBlock{ "lamp-light", Voxel::CubeData{ quat4(btQuaternion(-90.f * Math::DegToRadF, 0.f, 0.f)) } }, Vector::inty3(0, 3, 0)),
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


		// Sideview:
		//   #
		//  #
		// #
		// #
		// O
		// Topdown:
		// ##O
		// #
		// #
		const ParkourStep StepsToTheRight = ParkourStep
		{
			std::vector<std::pair<Voxel::NamedBlock, Vector::inty3>>{
				std::make_pair(Voxel::NamedBlock{ "wood", Voxel::CubeData{ quat4::identity() } }, Vector::inty3(0, 0, 0)),
				std::make_pair(Voxel::NamedBlock{ "wood", Voxel::CubeData{ quat4::identity() } }, Vector::inty3(0, 1, -1)),
				std::make_pair(Voxel::NamedBlock{ "wood", Voxel::CubeData{ quat4::identity() } }, Vector::inty3(0, 2, -2)),
				std::make_pair(Voxel::NamedBlock{ "wood", Voxel::CubeData{ quat4::identity() } }, Vector::inty3(1, 3, -2)),
				std::make_pair(Voxel::NamedBlock{ "wood", Voxel::CubeData{ quat4::identity() } }, Vector::inty3(2, 4, -2)),
			},
			std::vector<LookingPoint>{
				LookingPoint{ Vector::inty3(2, 4, -2), ParkourRightDirection },
			}
		};
		const ParkourStep StepsToTheLeft = ParkourStep
		{
			std::vector<std::pair<Voxel::NamedBlock, Vector::inty3>>{
				std::make_pair(Voxel::NamedBlock{ "wood", Voxel::CubeData{ quat4::identity() } }, Vector::inty3(0, 0, 0)),
				std::make_pair(Voxel::NamedBlock{ "wood", Voxel::CubeData{ quat4::identity() } }, Vector::inty3(0, 1, -1)),
				std::make_pair(Voxel::NamedBlock{ "wood", Voxel::CubeData{ quat4::identity() } }, Vector::inty3(0, 2, -2)),
				std::make_pair(Voxel::NamedBlock{ "wood", Voxel::CubeData{ quat4::identity() } }, Vector::inty3(-1, 3, -2)),
				std::make_pair(Voxel::NamedBlock{ "wood", Voxel::CubeData{ quat4::identity() } }, Vector::inty3(-2, 4, -2)),
			},
			std::vector<LookingPoint>{
				LookingPoint{ Vector::inty3(-2, 4, -2), ParkourLeftDirection },
			}
		};

		// Topdown:
		// X <-- exit
		// C <-- checkpoint
		// O <-- entrance
		const ParkourStep Checkpoint1 = ParkourStep
		{
			std::vector<std::pair<Voxel::NamedBlock, Vector::inty3>>{
				std::make_pair(Voxel::NamedBlock{ "wood", Voxel::CubeData{ quat4::identity() } }, Vector::inty3(0, 0, 0)),
				std::make_pair(Voxel::NamedBlock{ "checkpoint", Voxel::CubeData{ quat4::identity() } }, Vector::inty3(0, 0, -1)),
				std::make_pair(Voxel::NamedBlock{ "wood", Voxel::CubeData{ quat4::identity() } }, Vector::inty3(0, 0, -2)),
			},
			std::vector<LookingPoint>{
				LookingPoint{ Vector::inty3(0, 0, -2), ParkourForwardDirection },
			}
		};

		// Topdown:
		// #X# 
		// #C# 
		// #O# 
		const ParkourStep Checkpoint2 = ParkourStep
		{
			std::vector<std::pair<Voxel::NamedBlock, Vector::inty3>>{
				std::make_pair(Voxel::NamedBlock{ "wood", Voxel::CubeData{ quat4::identity() } }, Vector::inty3(0, 0, 0)),
				std::make_pair(Voxel::NamedBlock{ "checkpoint", Voxel::CubeData{ quat4::identity() } }, Vector::inty3(0, 0, -1)),
				std::make_pair(Voxel::NamedBlock{ "wood", Voxel::CubeData{ quat4::identity() } }, Vector::inty3(0, 0, -2)),
				std::make_pair(Voxel::NamedBlock{ "wood", Voxel::CubeData{ quat4::identity() } }, Vector::inty3(-1, 0, 0)),
				std::make_pair(Voxel::NamedBlock{ "wood", Voxel::CubeData{ quat4::identity() } }, Vector::inty3(-1, 0, -1)),
				std::make_pair(Voxel::NamedBlock{ "wood", Voxel::CubeData{ quat4::identity() } }, Vector::inty3(-1, 0, -2)),
				std::make_pair(Voxel::NamedBlock{ "wood", Voxel::CubeData{ quat4::identity() } }, Vector::inty3(1, 0, 0)),
				std::make_pair(Voxel::NamedBlock{ "wood", Voxel::CubeData{ quat4::identity() } }, Vector::inty3(1, 0, -1)),
				std::make_pair(Voxel::NamedBlock{ "wood", Voxel::CubeData{ quat4::identity() } }, Vector::inty3(1, 0, -2)),
			},
			std::vector<LookingPoint>{
				LookingPoint{ Vector::inty3(0, 0, -2), ParkourForwardDirection },
				LookingPoint{ Vector::inty3(-1, 0, -2), ParkourLeftDirection },
				LookingPoint{ Vector::inty3(1, 0, -2), ParkourRightDirection },
			}
		};

		// Now maybe some harder steps
		const ParkourStep DiagonalGapLeft = ParkourStep
		{
			std::vector<std::pair<Voxel::NamedBlock, Vector::inty3>>{
				std::make_pair(Voxel::NamedBlock{ "wood", Voxel::CubeData{ quat4::identity() } }, Vector::inty3(0, 0, 0)),
				std::make_pair(Voxel::NamedBlock{ "wood", Voxel::CubeData{ quat4::identity() } }, Vector::inty3(-2, 0,-2)),
			},
			std::vector<LookingPoint>{
				LookingPoint{ Vector::inty3(-2, 0, -2), ParkourLeftDirection },
			}
		};

		const ParkourStep DiagonalGapRight = ParkourStep
		{
			std::vector<std::pair<Voxel::NamedBlock, Vector::inty3>>{
				std::make_pair(Voxel::NamedBlock{ "wood", Voxel::CubeData{ quat4::identity() } }, Vector::inty3(0, 0, 0)),
				std::make_pair(Voxel::NamedBlock{ "wood", Voxel::CubeData{ quat4::identity() } }, Vector::inty3(2, 0,-2)),
			},
			std::vector<LookingPoint>{
				LookingPoint{ Vector::inty3(2, 0, -2), ParkourRightDirection },
			}
		};

		const ParkourStep ForwardGapSmallBlock = ParkourStep
		{
			std::vector<std::pair<Voxel::NamedBlock, Vector::inty3>>{
				std::make_pair(Voxel::NamedBlock{ "wood", Voxel::CubeData{ quat4::identity() } }, Vector::inty3(0, 0, 0)),
				std::make_pair(Voxel::NamedBlock{ "grass", Voxel::CubeData{ quat4::identity() } }, Vector::inty3(0, 0,-2)),
				std::make_pair(Voxel::NamedBlock{ "wood", Voxel::CubeData{ quat4::identity() } }, Vector::inty3(0, 0, -4)),
			},
			std::vector<LookingPoint>{
				LookingPoint{ Vector::inty3(0, 0, -4), ParkourForwardDirection },
			}
		};

		const ParkourStep TrickyTwoGaps = ParkourStep
		{
			std::vector<std::pair<Voxel::NamedBlock, Vector::inty3>>{
				std::make_pair(Voxel::NamedBlock{ "wood", Voxel::CubeData{ quat4::identity() } }, Vector::inty3(0, 0, 0)),
				std::make_pair(Voxel::NamedBlock{ "grass", Voxel::CubeData{ quat4::identity() } }, Vector::inty3(-1, 0, -3)),
				std::make_pair(Voxel::NamedBlock{ "grass", Voxel::CubeData{ quat4::identity() } }, Vector::inty3(1, 0, -5)),
				std::make_pair(Voxel::NamedBlock{ "wood", Voxel::CubeData{ quat4::identity() } }, Vector::inty3(0, 0, -8)),
			},
			std::vector<LookingPoint>{
				LookingPoint{ Vector::inty3(0, 0, -8), ParkourForwardDirection },
			}
		};

		const ParkourStep TrickyStairs = ParkourStep
		{
			std::vector<std::pair<Voxel::NamedBlock, Vector::inty3>>{
				std::make_pair(Voxel::NamedBlock{ "grass", Voxel::CubeData{ quat4::identity() } }, Vector::inty3(0, 0, 0)),
				std::make_pair(Voxel::NamedBlock{ "grass", Voxel::CubeData{ quat4::identity() } }, Vector::inty3(-1, 1, -1)),
				std::make_pair(Voxel::NamedBlock{ "grass", Voxel::CubeData{ quat4::identity() } }, Vector::inty3(1, 2, -2)),
				std::make_pair(Voxel::NamedBlock{ "grass", Voxel::CubeData{ quat4::identity() } }, Vector::inty3(-1, 3, -3)),
				std::make_pair(Voxel::NamedBlock{ "grass", Voxel::CubeData{ quat4::identity() } }, Vector::inty3(1, 4, -4)),
			},
			std::vector<LookingPoint>{
				LookingPoint{ Vector::inty3(1, 4, -4), ParkourForwardDirection },
			}
		};

		// Topdown:
		//   XX XX
		//    XXX
		//     X
		const ParkourStep TJunctionEasy = ParkourStep
		{
			std::vector<std::pair<Voxel::NamedBlock, Vector::inty3>> {
				std::make_pair(Voxel::NamedBlock{ "wood", Voxel::CubeData{ quat4::identity() } }, Vector::inty3{ 0, 0, 0 }),
				std::make_pair(Voxel::NamedBlock{ "wood", Voxel::CubeData{ quat4::identity() } }, Vector::inty3{ 0, 0, -1 }),
				std::make_pair(Voxel::NamedBlock{ "wood", Voxel::CubeData{ quat4::identity() } }, Vector::inty3{ 1, 0, -1 }),
				std::make_pair(Voxel::NamedBlock{ "wood", Voxel::CubeData{ quat4::identity() } }, Vector::inty3{ -1, 0, -1 }),
				std::make_pair(Voxel::NamedBlock{ "wood", Voxel::CubeData{ quat4::identity() } }, Vector::inty3{ 1, 0, -2 }),
				std::make_pair(Voxel::NamedBlock{ "wood", Voxel::CubeData{ quat4::identity() } }, Vector::inty3{ -1, 0, -2 }),
				std::make_pair(Voxel::NamedBlock{ "grass", Voxel::CubeData{ quat4::identity() } }, Vector::inty3{ 1, 1, -2 }),
				std::make_pair(Voxel::NamedBlock{ "grass", Voxel::CubeData{ quat4::identity() } }, Vector::inty3{ -1, 1, -2 }),
				std::make_pair(Voxel::NamedBlock{ "wood", Voxel::CubeData{ quat4::identity() } }, Vector::inty3{ 2, 1, -2 }),
				std::make_pair(Voxel::NamedBlock{ "wood", Voxel::CubeData{ quat4::identity() } }, Vector::inty3{ -2, 1, -2 }),

			},
			std::vector<LookingPoint>{
					LookingPoint{ Vector::inty3(+2, 1, -2), ParkourRightDirection },
					LookingPoint{ Vector::inty3(-2, 1, -2), ParkourLeftDirection },
			}
		};

		// Frontview:
		// X#
		//  ##
		//  ##O
		const ParkourStep ClimbingLampsEasy = ParkourStep
		{
			std::vector<std::pair<Voxel::NamedBlock, Vector::inty3>>{
				std::make_pair(Voxel::NamedBlock{ "wood", Voxel::CubeData{ quat4::identity() } }, Vector::inty3(0, 0, 0)),
				std::make_pair(Voxel::NamedBlock{ "wood", Voxel::CubeData{ quat4::identity() } }, Vector::inty3(-1, 0, 0)),
				std::make_pair(Voxel::NamedBlock{ "potted-hedge", Voxel::CubeData{ quat4::identity() } }, Vector::inty3(-1, 1, 0)),
				std::make_pair(Voxel::NamedBlock{ "wood", Voxel::CubeData{ quat4::identity() } }, Vector::inty3(-2, 0, 0)),
				std::make_pair(Voxel::NamedBlock{ "lamp-base", Voxel::CubeData{ quat4::identity() } }, Vector::inty3(-2, 1, 0)),
				std::make_pair(Voxel::NamedBlock{ "lamp-pole", Voxel::CubeData{ quat4::identity() } }, Vector::inty3(-2, 2, 0)),
				std::make_pair(Voxel::NamedBlock{ "lamp-bend", Voxel::CubeData{ quat4(btQuaternion(90.f * Math::DegToRadF, 0.f, 0.f)) } }, Vector::inty3(-2, 3, 0)),
				std::make_pair(Voxel::NamedBlock{ "lamp-light", Voxel::CubeData{ quat4(btQuaternion(90.f * Math::DegToRadF, 0.f, 0.f)) } }, Vector::inty3(-3, 3, 0)),
			},
			std::vector<LookingPoint>{
				LookingPoint{ Vector::inty3(-3, 3, 0), ParkourLeftDirection }
			}
		};

		// Topdown:
		//    X
		//    |
		//    |
		// #--C--X
		//    |
		//    |
		//    O
		const ParkourStep LampyCheckpoint = ParkourStep
		{
			std::vector<std::pair<Voxel::NamedBlock, Vector::inty3>>{
				std::make_pair(Voxel::NamedBlock{ "wood", Voxel::CubeData{ quat4::identity() } }, Vector::inty3(0, 0, 0)),
				std::make_pair(Voxel::NamedBlock{ "lamp-base", Voxel::CubeData{ quat4(btQuaternion(0.f, -90.f * Math::DegToRadF, 0.f)) } }, Vector::inty3(0, 0, -1)),
				std::make_pair(Voxel::NamedBlock{ "lamp-base", Voxel::CubeData{ quat4(btQuaternion(0.f, 90.f * Math::DegToRadF, 0.f)) } }, Vector::inty3(0, 0, -2)),
				std::make_pair(Voxel::NamedBlock{ "checkpoint", Voxel::CubeData{ quat4::identity() } }, Vector::inty3(0, 0, -3)),
				std::make_pair(Voxel::NamedBlock{ "lamp-base", Voxel::CubeData{ quat4(btQuaternion(0.f, -90.f * Math::DegToRadF, 0.f)) } }, Vector::inty3(0, 0, -4)),
				std::make_pair(Voxel::NamedBlock{ "lamp-base", Voxel::CubeData{ quat4(btQuaternion(0.f, 90.f * Math::DegToRadF, 0.f)) } }, Vector::inty3(0, 0, -5)),
				std::make_pair(Voxel::NamedBlock{ "wood", Voxel::CubeData{ quat4::identity() } }, Vector::inty3(0, 0, -6)),
				std::make_pair(Voxel::NamedBlock{ "lamp-base", Voxel::CubeData{ quat4(btQuaternion(0.f, 0.f, -90.f * Math::DegToRadF)) } }, Vector::inty3(1, 0, -3)),
				std::make_pair(Voxel::NamedBlock{ "lamp-base", Voxel::CubeData{ quat4(btQuaternion(0.f, 0.f, 90.f * Math::DegToRadF)) } }, Vector::inty3(2, 0, -3)),
				std::make_pair(Voxel::NamedBlock{ "wood", Voxel::CubeData{ quat4::identity() } }, Vector::inty3(3, 0, -3)),
				std::make_pair(Voxel::NamedBlock{ "lamp-base", Voxel::CubeData{ quat4(btQuaternion(0.f, 0.f, 90.f * Math::DegToRadF)) } }, Vector::inty3(-1, 0, -3)),
				std::make_pair(Voxel::NamedBlock{ "lamp-base", Voxel::CubeData{ quat4(btQuaternion(0.f, 0.f, -90.f * Math::DegToRadF)) } }, Vector::inty3(-2, 0, -3)),
				std::make_pair(Voxel::NamedBlock{ "wood", Voxel::CubeData{ quat4::identity() } }, Vector::inty3(-3, 0, -3)),
				std::make_pair(Voxel::NamedBlock{ "lamp-base", Voxel::CubeData{ quat4::identity() } }, Vector::inty3(-3, 1, -3)),
				std::make_pair(Voxel::NamedBlock{ "lamp-pole", Voxel::CubeData{ quat4::identity() } }, Vector::inty3(-3, 2, -3)),
				std::make_pair(Voxel::NamedBlock{ "lamp-bend", Voxel::CubeData{ quat4(btQuaternion(-90.f * Math::DegToRadF, 0.f, 0.f)) } }, Vector::inty3(-3, 3, -3)),
				std::make_pair(Voxel::NamedBlock{ "lamp-light", Voxel::CubeData{ quat4(btQuaternion(-90.f * Math::DegToRadF, 0.f, 0.f)) } }, Vector::inty3(-2, 3, -3)),
			},
			std::vector<LookingPoint>{
				LookingPoint{ Vector::inty3(0, 0, -6), ParkourForwardDirection },
				LookingPoint{ Vector::inty3(3, 0, -3), ParkourRightDirection },
			}
		};

		// Topdown:
		//    X
		// 
		// 
		// X
		// 
		//      X
		// 
		// X
		//  
		// 
		//   O
		const ParkourStep TwoWayStraightMedium = ParkourStep
		{
			std::vector<std::pair<Voxel::NamedBlock, Vector::inty3>> {
				std::make_pair(Voxel::NamedBlock{ "wood", Voxel::CubeData{ quat4::identity() } }, Vector::inty3{ 0, 0, 0 }),
				std::make_pair(Voxel::NamedBlock{ "wood", Voxel::CubeData{ quat4::identity() } }, Vector::inty3{ -1, 0, -3 }),
				std::make_pair(Voxel::NamedBlock{ "wood", Voxel::CubeData{ quat4::identity() } }, Vector::inty3{ -1, 0, -7 }),
				std::make_pair(Voxel::NamedBlock{ "wood", Voxel::CubeData{ quat4::identity() } }, Vector::inty3{ 1, 0, -5 }),
				std::make_pair(Voxel::NamedBlock{ "wood", Voxel::CubeData{ quat4::identity() } }, Vector::inty3{ 0, 0, -10 }),

			},
			std::vector<LookingPoint>{
					LookingPoint{ Vector::inty3(0, 0, -8), ParkourForwardDirection },
			}
		};

		// Top down:
		//  X
		//  
		//  
		//  p  <-- plant
		//
		//
		//  O
		const ParkourStep PlantStairs = ParkourStep
		{
			std::vector<std::pair<Voxel::NamedBlock, Vector::inty3>> {
				std::make_pair(Voxel::NamedBlock{ "wood", Voxel::CubeData{ quat4::identity() } }, Vector::inty3{ 0, 0, 0 }),
				std::make_pair(Voxel::NamedBlock{ "potted-plant", Voxel::CubeData{ quat4::identity() } }, Vector::inty3{ 0, 2, -3 }),
				std::make_pair(Voxel::NamedBlock{ "wood", Voxel::CubeData{ quat4::identity() } }, Vector::inty3{ 0, 4, -6 }),

			},
			std::vector<LookingPoint>{
					LookingPoint{ Vector::inty3(0, 4, -6), ParkourForwardDirection },
			}
		};

		// Topdown:     Side view:
		// p-p          p
		//
		//
		// p-p             p
		//
		//
		// p-p                p
		const ParkourStep PlantMonkeyBars = ParkourStep
		{
			std::vector<std::pair<Voxel::NamedBlock, Vector::inty3>> {
				std::make_pair(Voxel::NamedBlock{ "potted-plant", Voxel::CubeData{ quat4(btQuaternion(0.f, 0.f, 90.f * Math::DegToRadF)) } }, Vector::inty3{ -1, 0, 0 }),
				std::make_pair(Voxel::NamedBlock{ "lamp-pole", Voxel::CubeData{ quat4(btQuaternion(0.f, 0.f, -90.f * Math::DegToRadF)) } }, Vector::inty3{ 0, 0, 0 }),
				std::make_pair(Voxel::NamedBlock{ "potted-plant", Voxel::CubeData{ quat4(btQuaternion(0.f, 0.f, -90.f * Math::DegToRadF)) } }, Vector::inty3{ 1, 0, 0 }),
				std::make_pair(Voxel::NamedBlock{ "potted-plant", Voxel::CubeData{ quat4(btQuaternion(0.f, 0.f, 90.f * Math::DegToRadF)) } }, Vector::inty3{ -1, 2, -2 }),
				std::make_pair(Voxel::NamedBlock{ "lamp-pole", Voxel::CubeData{ quat4(btQuaternion(0.f, 0.f, -90.f * Math::DegToRadF)) } }, Vector::inty3{ 0, 2, -2 }),
				std::make_pair(Voxel::NamedBlock{ "potted-plant", Voxel::CubeData{ quat4(btQuaternion(0.f, 0.f, -90.f * Math::DegToRadF)) } }, Vector::inty3{ 1, 2, -2 }),
				std::make_pair(Voxel::NamedBlock{ "potted-plant", Voxel::CubeData{ quat4(btQuaternion(0.f, 0.f, 90.f * Math::DegToRadF)) } }, Vector::inty3{ -1, 4, -4 }),
				std::make_pair(Voxel::NamedBlock{ "lamp-pole", Voxel::CubeData{ quat4(btQuaternion(0.f, 0.f, -90.f * Math::DegToRadF)) } }, Vector::inty3{ 0, 4, -4 }),
				std::make_pair(Voxel::NamedBlock{ "potted-plant", Voxel::CubeData{ quat4(btQuaternion(0.f, 0.f, -90.f * Math::DegToRadF)) } }, Vector::inty3{ 1, 4, -4 }),

			},
			std::vector<LookingPoint>{
					LookingPoint{ Vector::inty3(0, 4, -4), ParkourForwardDirection },
			}
		};

		// Topdown:         Front view:
		//     xXX                 XXX   
		// X     XX               x  x
		//        X              
		//        x           X     X
		//      
		//   
		//          
		//      X            
		const ParkourStep BarrierLeftTurnHard = ParkourStep
		{
			std::vector<std::pair<Voxel::NamedBlock, Vector::inty3>> {
				std::make_pair(Voxel::NamedBlock{ "wood", Voxel::CubeData{ quat4::identity() } }, Vector::inty3{ 0, 0, 0 }),
				std::make_pair(Voxel::NamedBlock{ "wood", Voxel::CubeData{ quat4::identity() } }, Vector::inty3{ 0, 0, -6 }),
				std::make_pair(Voxel::NamedBlock{ "wood", Voxel::CubeData{ quat4::identity() } }, Vector::inty3{ -6, 0, -6 }),
				std::make_pair(Voxel::NamedBlock{ "grass", Voxel::CubeData{ quat4::identity() } }, Vector::inty3{ 1, 2, -4 }),
				std::make_pair(Voxel::NamedBlock{ "wood", Voxel::CubeData{ quat4::identity() } }, Vector::inty3{ 1, 3, -5 }),
				std::make_pair(Voxel::NamedBlock{ "wood", Voxel::CubeData{ quat4::identity() } }, Vector::inty3{ 1, 3, -6 }),
				std::make_pair(Voxel::NamedBlock{ "wood", Voxel::CubeData{ quat4::identity() } }, Vector::inty3{ 0, 3, -7 }),
				std::make_pair(Voxel::NamedBlock{ "wood", Voxel::CubeData{ quat4::identity() } }, Vector::inty3{ -1, 3, -7 }),
				std::make_pair(Voxel::NamedBlock{ "grass", Voxel::CubeData{ quat4::identity() } }, Vector::inty3{ -2, 2, -7 }),

			},
			std::vector<LookingPoint>{
					LookingPoint{ Vector::inty3(-6, 0, -6), ParkourLeftDirection },
			}
		};

		// Frontview:
		//  O---X
		//      |
		const ParkourStep LampDivingBoardHard = ParkourStep
		{
			std::vector<std::pair<Voxel::NamedBlock, Vector::inty3>>{
				std::make_pair(Voxel::NamedBlock{ "wood", Voxel::CubeData{ quat4::identity() } }, Vector::inty3(0, 0, 0)),
				std::make_pair(Voxel::NamedBlock{ "lamp-base", Voxel::CubeData{ quat4(btQuaternion(0, 0, -90.f * Math::DegToRadF)) } }, Vector::inty3(1, 0, 0)),
				std::make_pair(Voxel::NamedBlock{ "lamp-pole", Voxel::CubeData{ quat4(btQuaternion(0, 0, -90.f * Math::DegToRadF)) } }, Vector::inty3(2, 0, 0)),
				std::make_pair(Voxel::NamedBlock{ "lamp-pole", Voxel::CubeData{ quat4(btQuaternion(0, 0, -90.f * Math::DegToRadF)) } }, Vector::inty3(3, 0, 0)),
				std::make_pair(Voxel::NamedBlock{ "lamp-pole", Voxel::CubeData{ quat4(btQuaternion(0, 0, -90.f * Math::DegToRadF)) } }, Vector::inty3(4, 0, 0)),
				std::make_pair(Voxel::NamedBlock{ "lamp-bend", Voxel::CubeData{ quat4(btQuaternion(0, 0, -90.f * Math::DegToRadF) * btQuaternion(-90.f * Math::DegToRadF, 0.f, 0.f)) } }, Vector::inty3(5, 0, 0)),
				std::make_pair(Voxel::NamedBlock{ "lamp-light", Voxel::CubeData{ quat4(btQuaternion(0, 0, -90.f * Math::DegToRadF) * btQuaternion(-90.f * Math::DegToRadF, 0.f, 0.f)) } }, Vector::inty3(5, -1, 0)),
			},
			std::vector<LookingPoint>{
				LookingPoint{ Vector::inty3(5, 0, 0), ParkourRightDirection }
			}
		};

		// Topdown:
		// x <- rotated hedge
		// C
		// o <- rotated hedge
		const ParkourStep HedgeCheckpoint = ParkourStep
		{
			std::vector<std::pair<Voxel::NamedBlock, Vector::inty3>>{
				std::make_pair(Voxel::NamedBlock{ "potted-hedge", Voxel::CubeData{ quat4(btQuaternion(0.f, 90.f * Math::DegToRadF, 0.f)) } }, Vector::inty3(0, 0, 0)),
				std::make_pair(Voxel::NamedBlock{ "checkpoint", Voxel::CubeData{ quat4::identity() } }, Vector::inty3(0, 0, -1)),
				std::make_pair(Voxel::NamedBlock{ "potted-hedge", Voxel::CubeData{ quat4(btQuaternion(0.f, -90.f * Math::DegToRadF, 0.f)) } }, Vector::inty3(0, 0, -2)),
			},
			std::vector<LookingPoint>{
				LookingPoint{ Vector::inty3(0, 0, -2), ParkourForwardDirection }
			}
		};
	}

	const std::vector<ParkourStep> EasySteps{
		Steps::Checkpoint1,
		Steps::Checkpoint2,
		Steps::Checkpoint2,  // Duplicated to increase likelihood
		Steps::Long4LineStep,
		Steps::Fork2_2Step,
		Steps::CurvyStraightLeft,
		Steps::CurvyStraightRight,
		Steps::TripleSidewaysStep,
		Steps::TripleSidewaysStepWithSign,
		Steps::ClimbingLampsEasy,
		Steps::TJunctionEasy,
		Steps::LampyCheckpoint,
	};

	const std::vector<ParkourStep> MediumSteps{
		Steps::Checkpoint1,
		Steps::Checkpoint2,
		Steps::Checkpoint2,
		Steps::DecoratedSingleStep,
		Steps::StepsToTheLeft,
		Steps::StepsToTheRight,
		Steps::Fork2_2Step,
		Steps::TriExitGaps,
		Steps::HolyStairs,
		Steps::SingleBlockStepDecorLeft,
		Steps::SingleBlockStepDecorRight,
		Steps::TripleSidewaysStep,
		Steps::TripleSidewaysStepWithSign,
		Steps::LampDivingBoardHard,
		Steps::BarrierLeftTurnHard,
		Steps::HedgeCheckpoint,
		Steps::LampyCheckpoint,
	};

	const std::vector<ParkourStep> HardSteps{
		Steps::Checkpoint1,
		Steps::Checkpoint2,
		Steps::DecoratedSingleStep,
		Steps::StepsToTheLeft,
		Steps::StepsToTheRight,
		Steps::TriExitGaps,
		Steps::HolyStairs,
		Steps::SingleBlockStepDecorLeft,
		Steps::SingleBlockStepDecorRight,
		Steps::DiagonalGapLeft,
		Steps::DiagonalGapRight,
		Steps::ForwardGapSmallBlock,
		Steps::TrickyTwoGaps,
		Steps::TrickyStairs,
		Steps::PlantStairs,
		Steps::PlantMonkeyBars,
		Steps::HedgeCheckpoint,
		Steps::HedgeCheckpoint,
		Steps::LampDivingBoardHard,
		Steps::BarrierLeftTurnHard,
	};

	const std::array<Parkour::ParkourLevel, 5> Levels = {
		/*Parkour::ParkourLevel{
			Voxel::BlockCoord{ Voxel::ChunkCoord{ 0,0,0 }, Voxel::ChunkBlockCoord{0, 2, 5 }},
			Voxel::BlockFace::Forward,
			std::vector<ParkourStep>{ Steps::LampyCheckpoint, Steps::PlantMonkeyBars, Steps::HedgeCheckpoint, Steps::ClimbingLampsEasy, Steps::PlantStairs, Steps::LampDivingBoardHard, Steps::TJunctionEasy, Steps::TwoWayStraightMedium, Steps::BarrierLeftTurnHard },
			std::vector<std::pair<float, int>>{ std::make_pair(0.5f, 1), std::make_pair(1.0f, 2), std::make_pair(0.5f, 3) },
			std::vector<std::pair<float, int>>{ std::make_pair(0.25f, 0), std::make_pair(0.75f, 1) },
			25,
			Voxel::BlockCoord{},
			Voxel::BlockFace{}
			},*/
		Parkour::ParkourLevel{
			Voxel::BlockCoord{ Voxel::ChunkCoord{ 0,0,0 }, Voxel::ChunkBlockCoord{0, 2, 5 }},
			Voxel::BlockFace::Forward,
			EasySteps,
			std::vector<std::pair<float, int>>{ std::make_pair(0.5f, 1), std::make_pair(1.0f, 2), std::make_pair(0.5f, 3) },
			std::vector<std::pair<float, int>>{ std::make_pair(0.25f, 0), std::make_pair(0.75f, 1) },
			25,
			Voxel::BlockCoord{},
			Voxel::BlockFace{}
			},
		Parkour::ParkourLevel{
			Voxel::BlockCoord::Origin(),
			Voxel::BlockFace::Forward,
			MediumSteps,
			std::vector<std::pair<float, int>>{ std::make_pair(0.25f, 1), std::make_pair(0.5f, 2), std::make_pair(1.5f, 3), std::make_pair(0.15f, 4) },
			std::vector<std::pair<float, int>>{ std::make_pair(0.5f, 0), std::make_pair(0.5f, 1), std::make_pair(0.2f, 2) },
			50,
			Voxel::BlockCoord{ Voxel::ChunkCoord{ 0, 0, 0 }, { 0, 2, 0 } },
			Parkour::ParkourForwardDirection
			},
		Parkour::ParkourLevel{
			Voxel::BlockCoord::Origin(),
			Voxel::BlockFace::Forward,
			HardSteps,
			std::vector<std::pair<float, int>>{ std::make_pair(0.25f, 1), std::make_pair(0.5f, 2), std::make_pair(1.5f, 3), std::make_pair(0.15f, 4) },
			std::vector<std::pair<float, int>>{ std::make_pair(0.5f, 0), std::make_pair(0.5f, 1), std::make_pair(0.2f, 2) },
			50,
			Voxel::BlockCoord{ Voxel::ChunkCoord{ 0, 0, 0 }, { 0, 2, 0 } },
			Parkour::ParkourForwardDirection
			},
	};
}