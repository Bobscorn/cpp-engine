#pragma once

#include "Parkour.h"

#include <array>

namespace Parkour
{
	namespace Steps
	{
		// Topdown:
		// X
		// X
		// X
		// O
		extern const ParkourStep Long4LineStep;

		// Topdown:
		// X X
		// X X
		//  X
		//  O
		extern const ParkourStep Fork2_2Step;

		// Topdown:
		//   X
		// X   X
		//   O
		extern const ParkourStep TriExitGaps;

		// Sideview:
		//         XX
		//     XX
		// XX
		extern const ParkourStep HolyStairs;

		// Topdown:
		//      X
		//     X
		//   XX
		//  X
		// O
		extern const ParkourStep CurvyStraightRight;

		// Topdown:
		// X
		//  X
		//   XX
		//     X
		//      O
		extern const ParkourStep CurvyStraightLeft;

		// Topdown:
		// #O
		extern const ParkourStep SingleBlockStepDecorLeft;

		// Topdown:
		// O#
		extern const ParkourStep SingleBlockStepDecorRight;

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
		extern const ParkourStep StepsToTheRight;
		extern const ParkourStep StepsToTheLeft;

		// Topdown:
		// X <-- exit
		// C <-- checkpoint
		// O <-- entrance
		extern const ParkourStep Checkpoint1;
	}

	extern const std::array<Parkour::ParkourLevel, 5> Levels;
}

