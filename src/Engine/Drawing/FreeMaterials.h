#pragma once

#include "Graphics3D.h"

constexpr Material ShinyGreyBall = Material({ 0.02f, 0.02f, 0.02f, 1.f }, { 0.f, 0.f, 0.f, 1.f }, { 0.7f, 0.7f, 0.7f, 1.f }, { 0.3f, 0.3f, 0.3f, 20.f }, 1.f, 0);
constexpr Material Mat2 = Material();
constexpr Material Mat3 = Material();
constexpr Material Mat4 = Material();
constexpr Material Mat5 = Material();
constexpr Material Mat6 = Material();
constexpr Material Mat7 = Material();
constexpr Material Mat8 = Material();

constexpr Material FreeMats[8] = 
{
	ShinyGreyBall,
	Mat2,
	Mat3,
	Mat4,
	Mat5,
	Mat6,
	Mat7,
	Mat8
};