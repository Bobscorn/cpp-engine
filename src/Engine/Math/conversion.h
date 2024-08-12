#pragma once

#include "matrix.h"
#include "quat4.h"

namespace Math
{
	inline Matrixy4x4 quatToMatrix(const quat4& q)
	{
		float n = q.length2();
		float s = 0.f;
		if (n != 0.f)
			s = 2.f / n;

		float wx = s * w * x, wy = s * w * y, wz = s * w * z;
		float xx = s * x * x, xy = s * x * y, xz = s * x * z;
		float yy = s * y * y, yz = s * y * z, zz = s * z * z;

		Matrixy4x4 out;
		out.m = { 1 - (yy + zz),		xy + wz,		xz - wy, 0,
						xy - wz,  1 - (xx + zz),		yz + wx, 0,
						xz + wy,		yz - wx,  1 - (xx + yy), 0,
							  0,			   0,			  0, 1 };
		return out;
	}

}
