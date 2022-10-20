#pragma once

#include <LinearMath/btQuaternion.h>

#include "floaty.h"
#include "inty.h"

#include <ostream>

// A quaternion struct
// under the hood it cheats and uses bullet physics math (via btQuaternion)
#pragma warning(push)
#pragma warning(disable: 4201) // nameless struct/union
struct quat4
{
	constexpr quat4() : quat4(identity()) {}
	quat4(quat4&& other) = default;
	quat4(const quat4& other) = default;

	constexpr quat4(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}
	quat4(btQuaternion quat) : x(quat.x()), y(quat.y()), z(quat.z()), w(quat.w()) {}

	static constexpr quat4 identity() { return quat4(0.f, 0.f, 0.f, 1.f); }

	quat4& operator=(const quat4& other) = default;
	quat4& operator=(quat4&& other) = default;

	inline operator btQuaternion() const { return btQuaternion(x, y, z, w); }

	inline constexpr bool operator==(const quat4& other) const { return x == other.x && y == other.y && z == other.z && w == other.w; }
	inline constexpr bool operator!=(const quat4& other) const { return !operator==(other); }

	inline bool approximately_equal(const quat4& other) const { return fabsf(x - other.x) < 0.0001f && fabsf(y - other.y) < 0.0001f && fabsf(z - other.z) < 0.0001f && fabsf(w - other.w) < 0.0001f; }

	inline quat4 operator*(const quat4& other) const { return btQuaternion(x, y, z, w) * (btQuaternion)other; }
	inline quat4 operator*(const btQuaternion& other) const { return btQuaternion(x, y, z, w) * (btQuaternion)other; }

	inline quat4& operator*=(const quat4& other) { return *this = *this * other; }
	inline quat4& operator*=(const btQuaternion& other) { return *this = *this * other; }

	floaty3 rotate(const floaty3& dir) const;
	Vector::inty3 rotate(const Vector::inty3& dir) const;

	friend std::ostream& operator<<(std::ostream& os, const quat4& q)
	{
		auto prec = os.precision();
		os.precision(3);
		os << "quat4(" << q.x << ", " << q.y << ", " << q.z << ", " << q.w << ")";
		os.precision(prec);
		return os;
	}

	union
	{
		struct
		{
			float x, y, z, w;
		};
		float m[4];
	};
};
#pragma warning(pop)

