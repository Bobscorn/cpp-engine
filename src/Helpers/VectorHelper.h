#pragma once

#pragma warning(push, 0)
#include <LinearMath/btMatrix3x3.h>
#include <LinearMath/btTransform.h>
#include <Bullet3Common/b3Vector3.h>
#include <LinearMath/btVector3.h>
#pragma warning(pop)

#include <assimp/vector3.h>
#include <assimp/matrix4x4.h>
#include <iostream>

#include "Helpers/SDLHelper.h"

#include <cmath>
#include <memory>
#include <cstring>

struct Double
{
	Double(const double &val) : val(val) {};

	double val;

	inline operator double() { return val; }
	inline double operator*(const double &other) { return val * other; }
	inline double operator/(const double &other) { return val / other; }
	inline double operator+(const double &other) { return val + other; }
	inline double operator-(const double &other) { return val + other; }
	inline double operator%(const double &other) { double thing = other; return modf(val, &thing); }
};

struct DOUBLE2
{
	double x;
	double y;

	inline bool operator==(const DOUBLE2 &right) const { return x == right.x && y == right.y; };

	inline DOUBLE2 operator*(const double factor) const { return { x * factor, y * factor }; }
	inline DOUBLE2 operator*(const DOUBLE2 &right) const { return { x * right.x, y * right.y }; }
	inline DOUBLE2 operator/(const double factor) const { return { x / factor, y / factor }; }
	inline DOUBLE2 operator+(const DOUBLE2 &right) const { return { x + right.x, y + right.y }; }
	inline DOUBLE2 operator-(const DOUBLE2 &right) const { return { x - right.x, y - right.y }; }

	constexpr DOUBLE2() : x(0.0), y(0.0) {};
	constexpr DOUBLE2(double _x, double _y) : x(_x), y(_y) {};
	explicit DOUBLE2(double *arr) : x(arr[0]), y(arr[1]) {};
	inline double Length() { return sqrt(x * x + y * y); }
};

struct DOUBLE3
{
	double x;
	double y;
	double z;

	inline bool operator==(const DOUBLE3 &right) const { return x == right.x && y == right.y && z == right.z; };

	inline DOUBLE3 operator*(const double factor) const { return { x * factor, y * factor, z * factor }; }
	inline DOUBLE3 operator/(const double factor) const { return { x / factor, y / factor, z / factor }; }
	inline DOUBLE3 operator+(const DOUBLE3 &right) const { return { x + right.x, y + right.y, z + right.z }; }
	inline DOUBLE3 operator-(const DOUBLE3 &right) const { return { x - right.x, y - right.y, z - right.z }; }

	constexpr DOUBLE3() : x(0.0), y(0.0), z(0.0) {};
	constexpr DOUBLE3(double _x, double _y, double _z) : x(_x), y(_y), z(_z) {};
	explicit DOUBLE3(double *arr) : x(arr[0]), y(arr[1]), z(arr[2]) {};
	inline double Length() { return sqrt(x * x + y * y + z * z); }
};

struct DOUBLE4
{
	double x;
	double y;
	double z;
	double w;

	inline bool operator==(const DOUBLE4 &right) const { return x == right.x && y == right.y && z == right.z && w == right.w; }

	inline DOUBLE4 operator*(const double factor) const { return { x * factor, y * factor, z * factor, w * factor }; }
	inline DOUBLE4 operator*(const DOUBLE2 &right) const { return { x * right.x, y * right.y, z * right.x, w * right.y }; }
	inline DOUBLE4 operator/(const double factor) const { return { x / factor, y / factor, z / factor, w / factor }; }
	inline DOUBLE4 operator+(const DOUBLE4 &right) const { return { x + right.x, y + right.y, z + right.z, w + right.w }; }
	inline DOUBLE4 operator-(const DOUBLE4 &right) const { return { x - right.x, y - right.y, z - right.z, w - right.w }; }

	constexpr DOUBLE4() : x(0.0), y(0.0), z(0.0), w(0.0) {};
	constexpr DOUBLE4(double _x, double _y, double _z, double _w) : x(_x), y(_y), z(_z), w(_w) {};
	explicit DOUBLE4(double *arr) : x(arr[0]), y(arr[1]), z(arr[2]), w(arr[3]) {};
	inline double Length() { return sqrt(x * x + y * y + z * z + w * w); }
};

// POD struct used to aggregate construct a DOUBLE2 eg { 0.0, 0.0 }
struct DOUBLE2Const
{
	double x, y;

	constexpr operator DOUBLE2() { return DOUBLE2(x, y); }
};

struct DOUBLE3Const
{
	double x, y, z;

	constexpr operator DOUBLE3() { return DOUBLE3(x, y, z); }
};

struct DOUBLE4Const
{
	double x, y, z, w;

	constexpr operator DOUBLE4() { return DOUBLE4(x, y, z, w); }
};

struct SizeU
{
	unsigned int Width = 0u, Height = 0u;
	bool operator==(const SizeU& other) { return Width == other.Width && Height == other.Height; }
	inline operator DOUBLE2() { return { double(Width), double(Height) }; }
};

struct PointU
{
	int x = 0u, y = 0u;
	bool operator==(const PointU &other) { return x == other.x && y == other.y; }
	inline operator SizeU() { return { (unsigned int)x, (unsigned int)y }; }
};

inline DOUBLE4 operator*(const DOUBLE4& lhs, const SizeU& rhs) { return { lhs.x * double(rhs.Width), lhs.y * double(rhs.Height), lhs.z * double(rhs.Width), lhs.w * double(rhs.Height) }; }

inline DOUBLE2 operator*(const double factor, const DOUBLE2 &d2) { return d2.operator*(factor); }
inline DOUBLE3 operator*(const double factor, const DOUBLE3 &d3) { return d3.operator*(factor); }
inline DOUBLE4 operator*(const double factor, const DOUBLE4 &d4) { return d4.operator*(factor); }
inline DOUBLE4 operator*(const DOUBLE2 &left, const DOUBLE4 &d4) { return d4.operator*(left); }

inline DOUBLE2 operator/(const double factor, const DOUBLE2 &d2) { return d2.operator/(factor); }
inline DOUBLE3 operator/(const double factor, const DOUBLE3 &d3) { return d3.operator/(factor); }
inline DOUBLE4 operator/(const double factor, const DOUBLE4 &d4) { return d4.operator/(factor); }

struct floaty2
{
	constexpr floaty2() : x(0.f), y(0.f) {}
	constexpr floaty2(float x, float y) : x(x), y(y) {}

	inline float DistanceFrom(const floaty2 &other) const noexcept
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

	float x{ 0.f }, y{ 0.f };
};

inline float Distance(const floaty2 &a, const floaty2 &b) noexcept
{
	return a.DistanceFrom(b);
}


struct floaty3
{
	floaty3() = default;
	constexpr floaty3(float x, float y, float z) : x(x), y(y), z(z) {}
	constexpr floaty3(const floaty3 &a) : x(a.x), y(a.y), z(a.z) {}
	constexpr floaty3(const DOUBLE3 &a) : x(static_cast<float>(a.x)), y(static_cast<float>(a.y)), z(static_cast<float>(a.z)) {}
	//floaty3(const DirectX::XMFLOAT3& a) : x(a.x), y(a.y), z(a.z) {}
	explicit constexpr floaty3(const b3Vector3& a) : x(a.x), y(a.y), z(a.z) {}
	explicit floaty3(const btVector3& a) : x(a.x()), y(a.y()), z(a.z()) {}
	inline operator b3Vector3() const { return b3MakeVector3(x, y, z); }
	inline operator btVector3() const { return { x, y, z }; }
	inline operator DOUBLE3() const { return { x, y, z }; }
	//inline operator DirectX::XMFLOAT3() { return { x, y, z }; }
	explicit constexpr floaty3(const aiVector3D &a) : x(a.x), y(a.y), z(a.z) {}

	inline float dot(const floaty3 &other) const
	{
		return x * other.x + y * other.y + z * other.z;
	}

	inline floaty3 cross(const floaty3 &other) const
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

	inline bool operator==(const floaty3 &other) const
	{
		return x == other.x && y == other.y && z == other.z;
	}

	inline floaty3 &operator=(const floaty3 &other)
	{
		x = other.x;
		y = other.y;
		z = other.z;
		return *this;
	}

	inline floaty3 operator+(const floaty3 &other) const
	{
		return { x + other.x, y + other.y, z + other.z };
	}

	inline floaty3 operator-(const floaty3 &other) const
	{
		return { x - other.y, y - other.y, z - other.z };
	}

	inline floaty3 operator-() const
	{
		return { -x, -y, -z };
	}

	inline floaty3 &operator-=(const floaty3 &other)
	{
		x -= other.x;
		y -= other.y;
		z -= other.z;
		return *this;
	}

	inline floaty3 &operator+=(const floaty3 &other)
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

	inline floaty3 &operator*=(float factor)
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
	
	inline floaty3 operator/(floaty3 b) const
	{
		return { x / b.x, y / b.y, z / b.z };
	}

	inline floaty3 &operator/=(float factor)
	{
		x /= factor;
		y /= factor;
		z /= factor;
		return *this;
	}

	inline floaty3 &operator/=(floaty3 b)
	{
		x /= b.x;
		y /= b.y;
		z /= b.z;
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
		if (mag < FLT_EPSILON && mag > -FLT_EPSILON)
			x = y = z = 0.f;
		else
		{
			x = x / mag;
			y = y / mag;
			z = z / mag;
		}
	}

	static inline floaty3 SafelyNormalized(const floaty3 &me)
	{
		floaty3 out = me;
		out.safenorm();
		return out;
	}

	static inline floaty3 Normalized(const floaty3 &me)
	{
		floaty3 out = me;
		out.normalize();
		return out;
	};

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

inline floaty3 operator*(float k, const floaty3 &vec)
{
	return vec * k;
}

struct SDL_Color;

struct floaty4
{
	floaty4() = default;
	constexpr floaty4(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}
	//floaty4(const DirectX::XMFLOAT4& a) : x(a.x), y(a.y), z(a.z), w(a.w) {};
	constexpr floaty4(const b3Vector3& a) : x(a.x), y(a.y), z(a.z), w(a.w) {}
	floaty4(const btQuaternion &a) : x(a.x()), y(a.y()), z(a.z()), w(a.w()) {}
	floaty4(const btVector3& a) : x(a.x()), y(a.y()), z(a.z()), w(a.w()) {}
	inline operator b3Vector4() { return b3MakeVector4(x, y, z, w); }
	inline operator btVector4() { return { x, y, z, w }; }
	inline operator btQuaternion() { return { x, y, z, w }; }
	floaty4(SDL_Color color);
	explicit constexpr floaty4(const aiVector3D &a) : x(a.x), y(a.y), z(a.z), w(0.f) {}

	inline float dot(const floaty4 &other) const
	{
		return x * other.x + y * other.y + z * other.z + w * other.w;
	}

	inline float magnitude() const
	{
		return sqrtf(x * x + y * y + z * z + w * w);
	}

	inline bool operator==(const floaty4 &other) const
	{
		return x == other.x && y == other.y && z == other.z && w == other.w;
	}

	inline floaty4 &operator=(const floaty4 &other)
	{
		x = other.x;
		y = other.y;
		z = other.z;
		w = other.w;
		return *this;
	}

	inline floaty4 operator+(const floaty4 &other) const
	{
		return { x + other.x, y + other.y, z + other.z, w + other.w };
	}

	inline floaty4 operator-(const floaty4 &other) const
	{
		return { x - other.y, y - other.y, z - other.z, w - other.w };
	}

	inline floaty4 operator-() const
	{
		return { -x, -y, -z, -w };
	}

	inline floaty4 &operator-=(const floaty4 &other)
	{
		x -= other.x;
		y -= other.y;
		z -= other.z;
		w -= other.w;
		return *this;
	}

	inline floaty4 &operator+=(const floaty4 &other)
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

	inline floaty4 &operator*=(float factor)
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

	inline floaty4 &operator/=(float factor)
	{
		return operator*=(1.f / factor);
	}

	inline void normalize()
	{
		float mag = magnitude();
		operator/=(mag);
	}

	static inline floaty4 Normalized(const floaty4 &me)
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

struct Matrixy2x2
{
	union
	{
		struct
		{
			float m11, m12;
			float m21, m22;
		};
		struct
		{
			float _11, _12;
			float _21, _22;
		};
		float m[2][2];
	};

	inline floaty2 TransformPoint(floaty2 point) const
	{
		return
		{
			point.x * m11 + point.y * m21,
			point.x * m12 + point.y * m22
		};
	}
};

inline floaty2 operator*(const floaty2 &a, const Matrixy2x2 &b)
{
	return b.TransformPoint(a);
}

struct Matrixy2x3
{
	union
	{
		struct
		{
			float m11, m12, dx, m21, m22, dy;
		};
		struct
		{
			float _11, _12, _13;
			float _21, _22, _23;
		};
		float m[2][3];
	};

	constexpr Matrixy2x3() : Matrixy2x3(Identity()) {}
	constexpr Matrixy2x3(float _11, float _12, float _13, float _21, float _22, float _23) : _11(_11), _12(_12), _13(_13), _21(_21), _22(_22), _23(_23) {}

	constexpr Matrixy2x3(const Matrixy2x3 &other) : Matrixy2x3(other._11, other._12, other._13, other._21, other._22, other._23) {}

	inline static constexpr Matrixy2x3 Identity() { return Matrixy2x3(1.f, 0.f, 0.f, 0.f, 1.f, 0.f); }

	static inline Matrixy2x3 Translation(float x, float y) noexcept
	{
		Matrixy2x3 translation;

		translation.m11 = 1.0; translation.m12 = 0.0;
		translation.m21 = 0.0; translation.m22 = 1.0;
		translation.dx = x; translation.dy = y;

		return translation;
	}
	
	static inline Matrixy2x3 Scale(floaty2 scale, floaty2 center = { 0.f, 0.f })
	{
		Matrixy2x3 scaled;

		scaled._11 = scale.x; scaled._12 = 0.0;
		scaled._21 = 0.0; scaled._22 = scale.y;
		scaled._13 = center.x - scale.x * center.x;
		scaled._23 = center.y - scale.y * center.y;

		return scaled;
	}
	
	static inline Matrixy2x3 RotationR(float angle)
	{
		Matrixy2x3 rotation;

		rotation._11 = cosf(angle); rotation._12 = sinf(angle);
		rotation._21 = -sinf(angle); rotation._22 = cosf(angle);

		return rotation;
	}

	static inline Matrixy2x3 RotationR(float angle, floaty2 center)
	{
		Matrixy2x3 rotation = Translation(-center.x, -center.y);

		// Rotating with matrix like 
		//  _              _
		// | cos(θ), sin(θ) |
		// |_-sin(θ),cos(θ)_|

		rotation = rotation * RotationR(angle);
		
		return rotation * Translation(center.x, center.y);
	}
		
	inline float Determinant() const noexcept
	{
		return (_11 * _22) - (_12 * _21);
	}

	inline bool IsInvertible() const noexcept
	{
		return Determinant() != 0.f;
	}

	inline bool Invert() noexcept
	{
		float det = Determinant();
		if (det == 0.f)
			return false;
		
		float invdet = 1.f / det;

		Matrixy2x3 minv; // inverse of matrix m
		minv.m[0][0] = (m[1][1] * 1.f     - 0.f     * m[1][2]) * invdet;
		minv.m[0][1] = (m[0][2] * 0.f     - m[0][1] * 1.f    ) * invdet;
		minv.m[0][2] = (m[0][1] * m[1][2] - m[0][2] * m[1][1]) * invdet;
		minv.m[1][0] = (m[1][2] * 0.f     - m[1][0] * 1.f    ) * invdet;
		minv.m[1][1] = (m[0][0] * 1.f     - m[0][2] * 0.f    ) * invdet;
		minv.m[1][2] = (m[1][0] * m[0][2] - m[0][0] * m[1][2]) * invdet;
		*this = minv;

		return true;
	}

	inline bool IsIdentity() const
	{
		return _11 == 1.f && _12 == 0.f
			&& _21 == 0.f && _22 == 1.f
			&& _13 == 0.f && _23 == 0.f;
	}

	inline Matrixy2x2 As2x2() const noexcept
	{
		return { m11, m12, m21, m22 };
	}

	inline void SetProduct(const Matrixy2x3 &a, const Matrixy2x3 &b) noexcept
	{
		_11 = a._11 * b._11 + a._12 * b._21;
		_12 = a._11 * b._12 + a._12 * b._22;
		_21 = a._21 * b._11 + a._22 * b._21;
		_22 = a._21 * b._12 + a._22 * b._22;
		_13 = a._13 * b._11 + a._23 * b._21 + b._13;
		_23 = a._13 * b._12 + a._23 * b._22 + b._23;
	}
		
	inline Matrixy2x3 operator*(const Matrixy2x3 &other) const
	{
		Matrixy2x3 out;
		out._11 = this->_11 * other._11 + this->_12 * other._21;
		out._12 = this->_11 * other._12 + this->_12 * other._22;
		out._21 = this->_21 * other._11 + this->_22 * other._21;
		out._22 = this->_21 * other._12 + this->_22 * other._22;
		out._13 = this->_13 * other._11 + this->_23 * other._21 + other._13;
		out._23 = this->_13 * other._12 + this->_23 * other._22 + other._23;
		return out;
	}

	inline Matrixy2x3& operator*=(const Matrixy2x3 &other)
	{
		Matrixy2x3 copy = *this;
		_11 = copy._11 * other._11 + copy._12 * other._21;
		_12 = copy._11 * other._12 + copy._12 * other._22;
		_21 = copy._21 * other._11 + copy._22 * other._21;
		_22 = copy._21 * other._12 + copy._22 * other._22;
		_13 = copy._13 * other._11 + copy._23 * other._21 + other._13;
		_23 = copy._13 * other._12 + copy._23 * other._22 + other._23;
		return *this;
	}

	inline floaty2 TransformPoint(floaty2 point) const
	{
		floaty2 result =
		{
			point.x * m11 + point.y * m21 + dx,
			point.x * m12 + point.y * m22 + dy
		};

		return result;
	}

	inline PointRect TransformRect(PointRect rect)
	{
		floaty2 tmpa, tmpb;
		tmpa = TransformPoint({ rect.left, rect.top });
		tmpb = TransformPoint({ rect.right, rect.bottom });
		return { tmpa.x, tmpa.y, tmpb.x, tmpb.y };
	}
};

struct Matrixy3x3
{
	union
	{
		struct
		{
			float m11, m21, m31;
			float m12, m22, m32;
			float m13, m23, m33;
		};
		float m[3][3];
		float ma[9];
	};

	Matrixy3x3() = default;
	constexpr Matrixy3x3(float _11, float _12, float _13, float _21, float _22, float _23, float _31, float _32, float _33) :
		m11(_11), m12(_12), m13(_13),
		m21(_21), m22(_22), m23(_23),
		m31(_31), m32(_32), m33(_33) {}

	constexpr Matrixy3x3(const Matrixy3x3 &other) : Matrixy3x3(other.m11, other.m12, other.m13, other.m21, other.m22, other.m23, other.m31, other.m32, other.m33) {}
	Matrixy3x3(const btMatrix3x3 &other) : Matrixy3x3(other[0].getX(), other[0].getY(), other[0].getZ(), other[1].getX(), other[1].getY(), other[1].getZ(), other[2].getX(), other[2].getY(), other[2].getZ()) {}

	inline Matrixy3x3 &operator=(const Matrixy3x3 &other)
	{
		auto &o = other;
		m11 = o.m11; m12 = o.m12; m13 = o.m13;
		m21 = o.m21; m22 = o.m22; m23 = o.m23;
		m31 = o.m31; m32 = o.m32; m33 = o.m33;
		return *this;
	}

	Matrixy3x3 operator*(const Matrixy3x3 &other);

	inline floaty3 Transform(const floaty3 &vec) { return Transform(vec, *this); }
	static floaty3 Transform(const floaty3 &vec, const Matrixy3x3 &mat);

	static Matrixy3x3 Scale(float x, float y, float z);
	static Matrixy3x3 RotationAxisR(floaty3 axis, float angle);
	static Matrixy3x3 RotationAxisD(floaty3 axis, float angle);
	static Matrixy3x3 RotationAxisNormR(floaty3 axis, float angle); // Assume axis vector is already normalized
	static Matrixy3x3 RotationAxisNormD(floaty3 axis, float angle); // Assume axis vector is already normalized
	static Matrixy3x3 RotationR(floaty3 by);
	static Matrixy3x3 RotationD(floaty3 by);
	static Matrixy3x3 RotationR(float x, float y, float z);
	static Matrixy3x3 RotationD(float x, float y, float z);
	static Matrixy3x3 RotationX(float x); // Radians
	static Matrixy3x3 RotationY(float y); // Radians
	static Matrixy3x3 RotationZ(float z); // Radians
	static Matrixy3x3 ReflectionOrigin(floaty3 along);
	static Matrixy3x3 Transpose(const Matrixy3x3 &mat);
};

struct Matrixy4x4
{
	union
	{
		struct
		{
			float m11, m21, m31, m41;
			float m12, m22, m32, m42;
			float m13, m23, m33, m43;
			float dx,  dy,  dz,  m44;
		};
		struct
		{
			float _11, _21, _31, _41;
			float _12, _22, _32, _42;
			float _13, _23, _33, _43;
			float _14, _24, _34, _44;
		};
		float m[4][4];
		float ma[16];
	};

	Matrixy4x4() : Matrixy4x4(Identity()) {}
	Matrixy4x4(const float *data) { memcpy(ma, data, sizeof(float) * 16); }
	constexpr Matrixy4x4(float _11, float _12, float _13, float _14, float _21, float _22, float _23, float _24, float _31, float _32, float _33, float _34, float _41, float _42, float _43, float _44)
		:
		_11(_11), _12(_12), _13(_13), _14(_14),
		_21(_21), _22(_22), _23(_23), _24(_24),
		_31(_31), _32(_32), _33(_33), _34(_34),
		_41(_41), _42(_42), _43(_43), _44(_44) {}
	
	Matrixy4x4(const Matrixy4x4 &other) : ma
	{
		other.ma[0], other.ma[1], other.ma[2], other.ma[3], 
		other.ma[4], other.ma[5], other.ma[6], other.ma[7], 
		other.ma[8], other.ma[9], other.ma[10], other.ma[11], 
		other.ma[12], other.ma[13], other.ma[14], other.ma[15]
	} {}
	
	Matrixy4x4(Matrixy4x4 &&other) noexcept : ma
	{
		other.ma[0], other.ma[1], other.ma[2], other.ma[3],
		other.ma[4], other.ma[5], other.ma[6], other.ma[7],
		other.ma[8], other.ma[9], other.ma[10], other.ma[11],
		other.ma[12], other.ma[13], other.ma[14], other.ma[15]
	} {}
	
	
	Matrixy4x4(const Matrixy2x3 &mat) : ma
	{
		mat.m11, mat.m21, 0.f, 0.f,
		mat.m12, mat.m22, 0.f, 0.f,
		0.f,     0.f,     1.f, 0.f,
		mat.dx,  mat.dy,  0.f, 1.f
	} {}

	Matrixy4x4(const Matrixy3x3 &mat) : ma
	{
		mat.m11, mat.m21, mat.m31, 0.f,
		mat.m12, mat.m22, mat.m32, 0.f,
		mat.m13, mat.m23, mat.m33, 0.f,
		0.f,     0.f,     0.f,     1.f
	} {}

	static constexpr Matrixy4x4 AssimpTrans()
	{
		return { 0.f, 0.f, 1.f, 0.f,
				 1.f, 0.f, 0.f, 0.f,
				 0.f, 1.f, 0.f, 0.f,
				 0.f, 0.f, 0.f, 1.f };
	}

	Matrixy4x4(const aiMatrix4x4 & mat)
	{
		if constexpr (sizeof(Matrixy4x4) == sizeof(aiMatrix4x4))
			std::memcpy(ma, &mat, sizeof(Matrixy4x4));
		else
		{
			_11 = mat.a1;
			_21 = mat.a2;
			_31 = mat.a3;
			_41 = mat.a4;
			_12 = mat.b1;
			_22 = mat.b2;
			_32 = mat.b3;
			_42 = mat.b4;
			_13 = mat.c1;
			_23 = mat.c2;
			_33 = mat.c3;
			_43 = mat.c4;
			_14 = mat.d1;
			_24 = mat.d2;
			_34 = mat.d3;
			_44 = mat.d4;
		}
	}

	inline Matrixy3x3 As3x3() const
	{
		return { m11, m12, m13, m21, m22, m23, m31, m32, m33 };
	}

	inline bool IsIdentity() const noexcept
	{
		return m11 == 1.f
			&& m12 == 0.f
			&& m13 == 0.f
			&& dx == 0.f
			&& m21 == 0.f
			&& m22 == 1.f
			&& m23 == 0.f
			&& dy == 0.f
			&& m31 == 0.f
			&& m32 == 0.f
			&& m33 == 1.f
			&& dz == 0.f
			&& m41 == 0.f
			&& m42 == 0.f
			&& m43 == 0.f
			&& m44 == 1.f;
	}

	inline bool IsBlank() const noexcept
	{
		for (auto &e : ma)
			if (e != 0.f)
				return false;
		return true;
	}

	inline Matrixy4x4 &operator=(const Matrixy4x4 &other)
	{
		memcpy(ma, other.ma, sizeof(float) * 16);
		return *this;
	}

	inline Matrixy4x4 &operator=(const Matrixy2x3 &other)
	{
		Matrixy4x4 tmp(other);
		return operator=(tmp);
	}

	inline Matrixy4x4 &operator=(const Matrixy3x3 &other)
	{
		Matrixy4x4 tmp(other);
		return operator=(tmp);
	}

	static Matrixy4x4 Multiply(const Matrixy4x4 &a, const Matrixy4x4 &b); // Effectively: Create matrix that Applies matrix B, then matrix A
	inline static Matrixy4x4 MultiplyE(const Matrixy4x4 &a, const Matrixy4x4 &b) { return Multiply(b, a); } // Multiply A with B to create a matrix that applies A then B
	inline floaty3 Transform(const floaty3 &vec) const { return Transform(vec, *this); }
	inline floaty4 Transform(const floaty4 &vec) const { return Transform(vec, *this); }
	static floaty3 Transform(const floaty3 &vec, const Matrixy4x4 &mat);
	static floaty4 Transform(const floaty4 &vec, const Matrixy4x4 &mat);
	inline floaty4 TransformNormal(const floaty4 &vec) const { floaty3 out = TransformNormal({ vec.x, vec.y, vec.z }); return { out.x, out.y, out.z, 1.f }; }
	inline floaty3 TransformNormal(const floaty3 &vec) const { return TransformNormal(vec, *this); }
	static floaty3 TransformNormal(const floaty3 &vec, const Matrixy4x4 &mat);

	constexpr static inline Matrixy4x4 Identity()
	{
		return
			Matrixy4x4(
				1.f, 0.f, 0.f, 0.f,
				0.f, 1.f, 0.f, 0.f,
				0.f, 0.f, 1.f, 0.f,
				0.f, 0.f, 0.f, 1.f
			);
	}

	static Matrixy4x4 RotationAxisR(floaty3 axis, float angle); // Radians
	static Matrixy4x4 RotationAxisD(floaty3 axis, float angle); // Degrees
	static Matrixy4x4 Reflection(floaty3 along, floaty3 origin);
	static Matrixy4x4 Transpose(const Matrixy4x4 &mat);
	static bool Inverted(Matrixy4x4 in, Matrixy4x4 &out);
	inline static Matrixy4x4 InvertedOrIdentity(Matrixy4x4 in)
	{
		Matrixy4x4 out = in;
		if (out.Invert())
			return out;
		else
			return Matrixy4x4::Identity();
	}
	bool Invert();

	static inline Matrixy4x4 OrthoProject(float width, float height, float nz, float fz)
	{
		// asumed r-l = width , t-b = height
		Matrixy4x4 out;
		out.m[0][0] = 2.f / width; out.m[0][1] = 0;              out.m[0][2] = 0;                      out.m[0][3] = 0;
		out.m[1][0] = 0;           out.m[1][1] = 2.f / height;   out.m[1][2] = 0;                      out.m[1][3] = 0;
		out.m[2][0] = 0;           out.m[2][1] = 0;              out.m[2][2] = -2.f / (fz - nz);       out.m[2][3] = 0;
		out.m[3][0] = 0;           out.m[3][1] = 0;              out.m[3][2] = -(fz + nz) / (fz - nz); out.m[3][3] = 1.;
		return out;
	}

	static Matrixy4x4 LookAt(floaty3 eye, floaty3 target, floaty3 up);

	static Matrixy4x4 PerspectiveFovD(float fov, float aspect, float nearZ, float farZ); // Fov in degrees (converted to radians)
	static Matrixy4x4 PerspectiveFovR(float fov, float aspect, float nearZ, float farZ); // Fov in radians

	static inline Matrixy4x4 Translate(float x, float y, float z)
	{
		Matrixy4x4 out;
		out.dx = x;
		out.dy = y;
		out.dz = z;
		return out;
	}

	static inline Matrixy4x4 Translate(floaty3 by)
	{
		return Translate(by.x, by.y, by.z);
	}


	inline operator Matrixy2x3() const { return { ma[0], ma[1], ma[3], ma[4], ma[5], ma[7] }; }
	inline operator btTransform() const { btTransform out; out.setFromOpenGLMatrix(ma); return out; }
};

inline floaty2 operator*(const floaty2 &point, const Matrixy2x3 &matrix) noexcept
{
	return matrix.TransformPoint(point);
}

inline floaty2 operator*(float scalar, const floaty2 &vec)
{
	return { vec.x * scalar, vec.y * scalar };
}
inline floaty2 operator*(const floaty2 &vec, float scalar)
{
	return { vec.x * scalar, vec.y * scalar };
}

inline floaty2 operator+(const floaty2 &a, const floaty2 &b)
{
	return { a.x + b.x, a.y + b.y };
}
inline floaty2 operator-(const floaty2 &a)
{
	return { -a.x, -a.y };
}
inline floaty2 operator-(const floaty2 &a, const floaty2 &b)
{
	return a + (-b);
}



struct PointRect;
typedef PointRect Recty;


inline std::string AsString(const SDL_Rect &rect) { return std::string("SDLRect(x: ") + std::to_string(rect.x) + ", y: " + std::to_string(rect.y) + ", width: " + std::to_string(rect.w) + ", height: " + std::to_string(rect.h) + ")"; }
inline std::string AsString(const floaty2 &vec) { return std::string("vec2(x: ") + std::to_string(vec.x) + ", y: " + std::to_string(vec.y) + ")"; }
inline std::string AsString(const floaty3 &vec) { return std::string("vec3(x: ") + std::to_string(vec.x) + ", y: " + std::to_string(vec.y) + ", z: " + std::to_string(vec.z) + ")"; }
inline std::string AsString(const floaty4 &vec) { return std::string("vec4(x: ") + std::to_string(vec.x) + ", y: " + std::to_string(vec.y) + ", z: " + std::to_string(vec.z) + ", w: " + std::to_string(vec.w) + ")"; }
inline std::string AsString(const Matrixy2x2 &mat)
{
	return std::string("Mat2x2(") + "\n"
		+ std::to_string(mat.m11) + ", " + std::to_string(mat.m12) + "\n"
		+ std::to_string(mat.m21) + ", " + std::to_string(mat.m22) + "\n";
}
inline std::string AsString(const Matrixy2x3 &mat) 
{ 
	return std::string("Mat2x3(") + "\n"
		+ std::to_string(mat.m11) + ", " + std::to_string(mat.m12) + ", " + std::to_string(mat.dx) + "\n"
		+ std::to_string(mat.m21) + ", " + std::to_string(mat.m22) + ", " + std::to_string(mat.dy) + "\n";
}

inline std::string AsString(const Matrixy3x3 &mat)
{
	return std::string("Mat3x3(") + "\n"
		+ std::to_string(mat.m11) + ", " + std::to_string(mat.m12) + ", " + std::to_string(mat.m13) + "\n"
		+ std::to_string(mat.m21) + ", " + std::to_string(mat.m22) + ", " + std::to_string(mat.m23) + "\n"
		+ std::to_string(mat.m31) + ", " + std::to_string(mat.m32) + ", " + std::to_string(mat.m33) + "\n";
}

inline std::string AsString(const Matrixy4x4 &mat)
{
	return std::string("Mat4x4(") + "\n"
		+ std::to_string(mat.m11) + ", " + std::to_string(mat.m12) + ", " + std::to_string(mat.m13) + ", " + std::to_string(mat.dx) + "\n"
		+ std::to_string(mat.m21) + ", " + std::to_string(mat.m22) + ", " + std::to_string(mat.m23) + ", " + std::to_string(mat.dy) + "\n"
		+ std::to_string(mat.m31) + ", " + std::to_string(mat.m32) + ", " + std::to_string(mat.m33) + ", " + std::to_string(mat.dz) + "\n"
		+ std::to_string(mat.m41) + ", " + std::to_string(mat.m42) + ", " + std::to_string(mat.m43) + ", " + std::to_string(mat.m44) + "\n";
}

inline std::ostream &operator<<(std::ostream &a, const floaty3 &b)
{
	a << b.x << ", " << b.y << ", " << b.z;
	return a;
}

inline std::ostream &operator<<(std::ostream &a, const floaty4 &b)
{
	a << b.x << ", " << b.y << ", " << b.z << ", " << b.w;
	return a;
}

struct Frustum
{
	floaty3 Origin;
	floaty4 Orientation;

	float RightSlope;
	float LeftSlope;
	float TopSlope;
	float BottomSlope;
	float Near, Far;
};

void ComputeFrustum(Frustum *out, const Matrixy4x4 &projection);