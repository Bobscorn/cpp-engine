#pragma once

#pragma warning(push, 0)
#include <LinearMath/btMatrix3x3.h>
#include <LinearMath/btTransform.h>
#include <Bullet3Common/b3Vector3.h>
#include <LinearMath/btVector3.h>
#pragma warning(pop)

#include <iostream>

#include <cmath>
#include <memory>
#include <string>

#include "Math/matrix.h"
#include "Math/inty.h"

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
	inline DOUBLE4 operator*(const Vector::inty2 &right) const { return { x * double(right.x), y * double(right.y), z * double(right.x), w * double(right.y) }; }
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




struct DimensionRect;

#pragma warning(push)
#pragma warning(disable:4201)
struct PointRect
{
	union
	{
		struct
		{
			float x1, y1, x2, y2;
		};
		struct
		{
			float left, top, right, bottom;
		};
	};

	PointRect() : x1(0.f), y1(0.f), x2(0.f), y2(0.f) {}
	PointRect(float left, float top, float right, float bottom) : left(left), top(top), right(right), bottom(bottom) {}
	PointRect(const DimensionRect& rect);

	operator DimensionRect();
};
#pragma warning(pop)

struct DimensionRect
{
	DimensionRect() {}
	DimensionRect(float x, float y, float w, float h) : x(x), y(y), w(w), h(h) {}
	DimensionRect(const PointRect& rect) : x(rect.x1), y(rect.y1), w(rect.x2 - rect.x1), h(rect.y2 - rect.y1) {}

	inline DimensionRect& operator=(const PointRect& rect) { x = rect.x1; y = rect.y1; w = rect.x2 - rect.x1; h = rect.y2 - rect.y1; return *this; }

	float x{ 0.f }, y{ 0.f }, w{ 0.f }, h{ 0.f };

	inline operator PointRect() { return { x, y, x + w, y + h }; }
};

typedef DimensionRect Rect;
typedef PointRect Recty;


/// Erroring

inline std::string AsString(const PointRect& rect)
{
	return std::string("PRect(left: ") + std::to_string(rect.left) + ", top: " + std::to_string(rect.top) + ", right: " + std::to_string(rect.right) + ", bottom: " + std::to_string(rect.bottom) + ")";
}

inline std::string AsString(const DimensionRect& rect)
{
	return std::string("DRect(x: ") + std::to_string(rect.x) + ", y: " + std::to_string(rect.y) + ", width: " + std::to_string(rect.w) + ", height: " + std::to_string(rect.h) + ")";
}


//inline std::string AsString(const SDL_Rect &rect) { return std::string("SDLRect(x: ") + std::to_string(rect.x) + ", y: " + std::to_string(rect.y) + ", width: " + std::to_string(rect.w) + ", height: " + std::to_string(rect.h) + ")"; }
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
