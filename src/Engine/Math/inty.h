#pragma once

#include <functional>
#include <ostream>

namespace Vector
{
#pragma warning(push)
#pragma warning(disable:4201)
	struct inty4
	{
		union
		{
			struct
			{
				int x, y, z, w;
			};
			struct
			{
				int r, g, b, a;
			};
			int m[4];
		};

		inty4() = default;
		constexpr inty4(int x, int y, int z, int w) : x(x), y(y), z(z), w(w) {}
	};

	struct inty3
	{
		union
		{
			struct
			{
				int x, y, z;
			};
			struct
			{
				int r, g, b;
			};
			int m[3];
		};

		inty3() = default;
		constexpr inty3(int x, int y, int z) : x(x), y(y), z(z) {}

		inline constexpr bool operator==(const inty3& other) const { return x == other.x && y == other.y && z == other.z; }
		inline constexpr bool operator!=(const inty3& other) const { return !(*this == other); }

		inline constexpr inty3 operator+(const inty3& other) const { return inty3{ x + other.x, y + other.y, z + other.z }; }
		inline constexpr inty3 operator-(const inty3& other) const { return inty3{ x - other.x, y - other.y, z - other.z }; }
		inline constexpr inty3 operator-() const { return inty3{ -x, -y, -z }; }

		inline constexpr inty3 operator*(int scalar) const { return inty3{ x * scalar, y * scalar, z * scalar }; }
		inline constexpr inty3 operator/(int scalar) const { return inty3{ x / scalar, y / scalar, z / scalar }; }

		inline constexpr inty3 operator*(const inty3& other) const { return inty3{ x * other.x, y * other.y, z * other.z }; }
		inline constexpr inty3 operator/(const inty3& other) const { return inty3{ x / other.x, y / other.y, z / other.z }; }

		inline constexpr inty3& operator+=(const inty3& other) { x += other.x; y += other.y; z += other.z; return *this; }
		inline constexpr inty3& operator-=(const inty3& other) { x -= other.x; y -= other.y; z -= other.z; return *this; }

		friend std::ostream& operator<<(std::ostream& os, const inty3& v)
		{
			os << "inty3(" << v.x << ", " << v.y << ", " << v.z << ")";
			return os;
		}
	};

	struct inty2
	{
		union
		{
			struct
			{
				int x, y;
			};
			int m[2];
		};

		inty2() = default;
		constexpr inty2(int x, int y) : x(x), y(y) {}

		inline constexpr bool operator==(const inty2& other) const { return x == other.x && y == other.y; }
		inline constexpr bool operator!=(const inty2& other) const { return !(*this == other); }

	};
#pragma warning(pop)
}

namespace std
{
	template<>
	struct hash<Vector::inty3>
	{
		size_t operator()(const Vector::inty3& v) const
		{
			return hash<int>()(v.x) ^ hash<int>()(v.y) ^ hash<int>()(v.z);
		}
	};
}