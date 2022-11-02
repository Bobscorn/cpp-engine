#pragma once

#include <limits>

#include <assimp/vector3.h>

struct floaty2
{
	constexpr floaty2() : x(0.f), y(0.f) {}
	constexpr floaty2(float x, float y) : x(x), y(y) {}

	inline float DistanceFrom(const floaty2& other) const noexcept
	{
		floaty2 atob = { x - other.x, y - other.y };
		return sqrtf(atob.x * atob.x + atob.y * atob.y);
	}

	inline static floaty2 TLtoGL(floaty2 e, int width, int height) // Top Left (positive down, origin topleft) coords to GL coords (positive up, origin center)
	{
		float halfwidth = float(width) * 0.5f;
		float halfheight = float(height) * 0.5f;
		return TLtoGL(e, { halfwidth, halfheight });
	}

	inline static floaty2 TLtoGL(floaty2 e, floaty2 halfdims)
	{
		return { e.x - halfdims.x, halfdims.y - e.y };
	};

	inline static floaty2 TLtoGL(int x, int y, int width, int height)
	{
		return TLtoGL({ float(x), float(y) }, width, height);
	}

	inline static floaty2 GLtoTL(floaty2 e, int width, int height)
	{
		return { e.x + (float)width, -e.y + (float)height };
	}

	inline static floaty2 GLtoTL(floaty2 e, float width, float height)
	{
		return { e.x + width, -e.y + height };
	}

	inline bool operator==(const floaty2& other) const
	{
		return x == other.x
			&& y == other.y;
	}

	inline bool operator!=(const floaty2& other) const
	{
		return !(*this == other);
	}

	float x{ 0.f }, y{ 0.f };
};

inline float Distance(const floaty2& a, const floaty2& b) noexcept
{
	return a.DistanceFrom(b);
}

struct DOUBLE3;
class b3Vector3;
class btVector3;

#pragma warning(push)
#pragma warning(disable:4201)
struct floaty3
{
	floaty3() = default;
	constexpr floaty3(float x, float y, float z) : x(x), y(y), z(z) {}
	constexpr floaty3(const floaty3& a) : x(a.x), y(a.y), z(a.z) {}
	floaty3(const DOUBLE3& a);
	explicit floaty3(const b3Vector3& a);
	explicit floaty3(const btVector3& a);
	operator b3Vector3() const;
	operator btVector3() const;
	operator DOUBLE3() const;
	explicit floaty3(const aiVector3D& a);

	inline float dot(const floaty3& other) const
	{
		return x * other.x + y * other.y + z * other.z;
	}

	inline floaty3 cross(const floaty3& other) const
	{
		return { y * other.z - z * other.y, z * other.x - x * other.z, x * other.y - y * other.x };
	}

	inline float magnitude() const
	{
		return sqrtf(mag2());
	}

	inline constexpr float mag2() const
	{
		return x * x + y * y + z * z;
	}

	inline floaty3 as_abs() const
	{
		return floaty3{ fabsf(x), fabsf(y), fabsf(z) };
	}

	inline bool nearly_equal(floaty3 other) const
	{
		return fabsf(x - other.x) < 0.0001f && fabsf(y - other.y) < 0.0001f && fabsf(z - other.z) < 0.0001f;
	}

	inline bool operator==(const floaty3& other) const
	{
		return x == other.x && y == other.y && z == other.z;
	}

	inline floaty3& operator=(const floaty3& other)
	{
		x = other.x;
		y = other.y;
		z = other.z;
		return *this;
	}

	inline floaty3 operator+(const floaty3& other) const
	{
		return { x + other.x, y + other.y, z + other.z };
	}

	inline floaty3 operator-(const floaty3& other) const
	{
		return { x - other.x, y - other.y, z - other.z };
	}

	inline floaty3 operator-() const
	{
		return { -x, -y, -z };
	}

	inline floaty3& operator-=(const floaty3& other)
	{
		x -= other.x;
		y -= other.y;
		z -= other.z;
		return *this;
	}

	inline floaty3& operator+=(const floaty3& other)
	{
		x += other.x;
		y += other.y;
		z += other.z;
		return *this;
	}

	inline floaty3 operator*(float factor) const
	{
		return { x * factor, y * factor, z * factor };
	}

	inline floaty3& operator*=(float factor)
	{
		x *= factor;
		y *= factor;
		z *= factor;
		return *this;
	}

	inline floaty3 operator/(float factor) const
	{
		return { x / factor, y / factor,z / factor };
	}

	inline floaty3 operator/(floaty3 other) const
	{
		return { x / other.x, y / other.y, z / other.z };
	}

	inline floaty3& operator/=(float factor)
	{
		x /= factor;
		y /= factor;
		z /= factor;
		return *this;
	}

	inline floaty3& operator/=(floaty3 other)
	{
		x /= other.x;
		y /= other.y;
		z /= other.z;
		return *this;
	}

	inline void normalize()
	{
		float mag = magnitude();
		x = x / mag;
		y = y / mag;
		z = z / mag;
	}

	inline void safenorm()
	{
		float mag = magnitude();
		if (mag < std::numeric_limits<float>::epsilon() && mag > -std::numeric_limits<float>::epsilon())
			x = y = z = 0.f;
		else
		{
			x = x / mag;
			y = y / mag;
			z = z / mag;
		}
	}

	static inline floaty3 SafelyNormalized(const floaty3& me)
	{
		floaty3 out = me;
		out.safenorm();
		return out;
	}

	static inline floaty3 Normalized(const floaty3& me)
	{
		floaty3 out = me;
		out.normalize();
		return out;
	};

	inline operator floaty2() const { return floaty2{ x, y }; }

	union
	{
		struct
		{
			float x, y, z;
		};
		struct
		{
			float r, g, b;
		};
		float m[3];
	};
};

inline floaty3 operator*(float k, const floaty3& vec)
{
	return vec * k;
}

struct SDL_Color;
class btQuaternion;
class b3Vector4;
class btVector4;

struct floaty4
{
	floaty4() = default;
	constexpr floaty4(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}
	constexpr floaty4(floaty3 v, float w) : x(v.x), y(v.y), z(v.z), w(w) {}
	//floaty4(const DirectX::XMFLOAT4& a) : x(a.x), y(a.y), z(a.z), w(a.w) {};
	floaty4(const b3Vector4& a);
	floaty4(const btQuaternion& a);
	floaty4(const btVector3& a);
	operator b3Vector4() const;
	operator btVector4() const;
	operator btQuaternion() const;
	floaty4(SDL_Color color);
	explicit floaty4(const aiVector3D& a);

	inline float dot(const floaty4& other) const
	{
		return x * other.x + y * other.y + z * other.z + w * other.w;
	}

	inline float magnitude() const
	{
		return sqrtf(x * x + y * y + z * z + w * w);
	}

	inline bool operator==(const floaty4& other) const
	{
		return x == other.x && y == other.y && z == other.z && w == other.w;
	}

	inline floaty4& operator=(const floaty4& other)
	{
		x = other.x;
		y = other.y;
		z = other.z;
		w = other.w;
		return *this;
	}

	inline floaty4 operator+(const floaty4& other) const
	{
		return { x + other.x, y + other.y, z + other.z, w + other.w };
	}

	inline floaty4 operator-(const floaty4& other) const
	{
		return { x - other.y, y - other.y, z - other.z, w - other.w };
	}

	inline floaty4 operator-() const
	{
		return { -x, -y, -z, -w };
	}

	inline floaty4& operator-=(const floaty4& other)
	{
		x -= other.x;
		y -= other.y;
		z -= other.z;
		w -= other.w;
		return *this;
	}

	inline floaty4& operator+=(const floaty4& other)
	{
		x += other.x;
		y += other.y;
		z += other.z;
		w += other.w;
		return *this;
	}

	inline floaty4 operator*(float factor) const
	{
		return { x * factor, y * factor, z * factor, w * factor };
	}

	inline floaty4& operator*=(float factor)
	{
		x *= factor;
		y *= factor;
		z *= factor;
		w *= factor;
		return *this;
	}

	inline floaty4 operator/(float factor) const
	{
		float invfac = 1.f / factor;
		return { x * invfac, y * invfac, z * invfac, w * invfac };
	}

	inline floaty4& operator/=(float factor)
	{
		return operator*=(1.f / factor);
	}

	inline void normalize()
	{
		float mag = magnitude();
		operator/=(mag);
	}

	static inline floaty4 Normalized(const floaty4& me)
	{
		floaty4 out = me;
		out.normalize();
		return out;
	};

	union
	{
		struct
		{
			float x, y, z, w;
		};
		struct
		{
			float r, g, b, a;
		};
		float m[4];
	};
};

#pragma warning(pop)
