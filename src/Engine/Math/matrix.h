#pragma once

#include "floaty.h"
#include "inty.h"

#include <assimp/matrix4x4.h>

#include <cstring>

#pragma warning(push)
#pragma warning(disable: 4201) // nonstandard extension used : nameless struct/union
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

inline floaty2 operator*(const floaty2& a, const Matrixy2x2& b)
{
	return b.TransformPoint(a);
}

struct PointRect;

/**
* \brief A 2x3 matrix struct
*
* Matrixy2x3 is a 2x3 matrix struct used primarily for 2D transformations.
* It is capable of representing any typical 2D transformations like rotations, translations, scaling, shearing.
* Unlike Matrixy4x4 and Matrixy3x3, Matrixy2x3 (this class) is *row-major* and as such does not work smoothly with OpenGL.
* However OpenGL does not have an equivalent to a 2x3 matrix, this class is primarily intended for CPU based transforming.
*/
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

	constexpr Matrixy2x3(const Matrixy2x3& other) : Matrixy2x3(other._11, other._12, other._13, other._21, other._22, other._23) {}

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
		minv.m[0][0] = (m[1][1] * 1.f - 0.f * m[1][2]) * invdet;
		minv.m[0][1] = (m[0][2] * 0.f - m[0][1] * 1.f) * invdet;
		minv.m[0][2] = (m[0][1] * m[1][2] - m[0][2] * m[1][1]) * invdet;
		minv.m[1][0] = (m[1][2] * 0.f - m[1][0] * 1.f) * invdet;
		minv.m[1][1] = (m[0][0] * 1.f - m[0][2] * 0.f) * invdet;
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

	/**
	* Sets this matrix to the product of multiplying a with b
	* Unlike Matrixy4x4 and Matrixy3x3 operations, these multiplications apply *A then B*
	*
	* \param a is the matrix to apply the transformation of first
	* \param b is the matrix to apply the transformation of last
	*/
	inline void SetProduct(const Matrixy2x3& a, const Matrixy2x3& b) noexcept
	{
		_11 = a._11 * b._11 + a._12 * b._21;
		_12 = a._11 * b._12 + a._12 * b._22;
		_21 = a._21 * b._11 + a._22 * b._21;
		_22 = a._21 * b._12 + a._22 * b._22;
		_13 = a._13 * b._11 + a._23 * b._21 + b._13;
		_23 = a._13 * b._12 + a._23 * b._22 + b._23;
	}

	/**
	* Multiples this matrix with another matrix.
	* This produces a matrix that transforms something identically to transforming by A and then B
	*
	* \param other is the other to multiple with
	* \returns The matrix that applies this transformation and then the other transformation
	*
	* \remarks Unlike Matrixy3x3 and Matrixy4x4, Matrixy2x3 multiplication produces a matrix that applies *A then B* (not B then A)
	*/
	inline Matrixy2x3 operator*(const Matrixy2x3& other) const
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

	inline Matrixy2x3 operator*(const Vector::inty2& scale) const
	{
		Matrixy2x3 out;
		out._11 = this->_11;
		out._12 = this->_12;
		out._21 = this->_21;
		out._22 = this->_22;
		out._13 = this->_13 * (float)scale.x;
		out._23 = this->_23 * (float)scale.y;
		return out;
	}

	/**
	* The equivalent of *this = *this * other;
	* Applies this transformation then other's transformation
	*
	* \sa Matrixy2x3::operator*(const Matrixy2x3& other) const
	*/
	inline Matrixy2x3& operator*=(const Matrixy2x3& other)
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

	PointRect TransformRect(const PointRect& rect) const;
};

class btMatrix3x3;

/**
* \brief A column-major 3x3 matrix
*
* Matrixy3x3 is a 3x3 matrix class used for non-translation based 3D transformation (or otherwise).
* It is column-major to be consistent with Matrixy4x4 and OpenGL.
*/
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

	constexpr Matrixy3x3(const Matrixy3x3& other) : Matrixy3x3(other.m11, other.m12, other.m13, other.m21, other.m22, other.m23, other.m31, other.m32, other.m33) {}
	Matrixy3x3(const btMatrix3x3& other);

	inline Matrixy3x3& operator=(const Matrixy3x3& other)
	{
		auto& o = other;
		m11 = o.m11; m12 = o.m12; m13 = o.m13;
		m21 = o.m21; m22 = o.m22; m23 = o.m23;
		m31 = o.m31; m32 = o.m32; m33 = o.m33;
		return *this;
	}

	Matrixy3x3 operator*(const Matrixy3x3& other);

	inline floaty3 Transform(const floaty3& vec) { return Transform(vec, *this); }
	static floaty3 Transform(const floaty3& vec, const Matrixy3x3& mat);

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
	static Matrixy3x3 Transpose(const Matrixy3x3& mat);
};

class btTransform;

/**
* \brief A column-major 4x4 matrix struct
*
* Matrixy4x4 is a column-major 4x4 matrix class.
* It is column-major to interop with OpenGL as smoothly as possible.
*/
struct Matrixy4x4
{
	union
	{
		struct
		{
			float m11, m21, m31, m41;
			float m12, m22, m32, m42;
			float m13, m23, m33, m43;
			float dx, dy, dz, m44;
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
	Matrixy4x4(const float* data) { memcpy(ma, data, sizeof(float) * 16); }
	constexpr Matrixy4x4(float _11, float _12, float _13, float _14, float _21, float _22, float _23, float _24, float _31, float _32, float _33, float _34, float _41, float _42, float _43, float _44)
		:
		_11(_11), _12(_12), _13(_13), _14(_14),
		_21(_21), _22(_22), _23(_23), _24(_24),
		_31(_31), _32(_32), _33(_33), _34(_34),
		_41(_41), _42(_42), _43(_43), _44(_44) {}

	Matrixy4x4(const Matrixy4x4& other) : ma
	{
		other.ma[0], other.ma[1], other.ma[2], other.ma[3],
		other.ma[4], other.ma[5], other.ma[6], other.ma[7],
		other.ma[8], other.ma[9], other.ma[10], other.ma[11],
		other.ma[12], other.ma[13], other.ma[14], other.ma[15]
	} {}

	Matrixy4x4(Matrixy4x4&& other) noexcept : ma
	{
		other.ma[0], other.ma[1], other.ma[2], other.ma[3],
		other.ma[4], other.ma[5], other.ma[6], other.ma[7],
		other.ma[8], other.ma[9], other.ma[10], other.ma[11],
		other.ma[12], other.ma[13], other.ma[14], other.ma[15]
	} {}


	Matrixy4x4(const Matrixy2x3& mat) : ma
	{
		mat.m11, mat.m21, 0.f, 0.f,
		mat.m12, mat.m22, 0.f, 0.f,
		0.f,     0.f,     1.f, 0.f,
		mat.dx,  mat.dy,  0.f, 1.f
	} {}

	Matrixy4x4(const Matrixy3x3& mat) : ma
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

	Matrixy4x4(const aiMatrix4x4& mat)
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

	constexpr inline bool IsIdentity() const noexcept
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
		for (auto& e : ma)
			if (e != 0.f)
				return false;
		return true;
	}

	inline Matrixy4x4& operator=(const Matrixy4x4& other)
	{
		memcpy(ma, other.ma, sizeof(float) * 16);
		return *this;
	}

	inline Matrixy4x4& operator=(const Matrixy2x3& other)
	{
		Matrixy4x4 tmp(other);
		return operator=(tmp);
	}

	inline Matrixy4x4& operator=(const Matrixy3x3& other)
	{
		Matrixy4x4 tmp(other);
		return operator=(tmp);
	}
	/**
	* \brief Multiples matrix A with B
	*
	*  Multiplies 2 matrices together to produce a matrix that applies b and then a
	*
	* @param a is the matrix whose transformation is applied after b's
	* @param b is the matrix whose transformation is applied first
	* @return Returns the matrix that applies the transformation B then A
	*/
	static Matrixy4x4 Multiply(const Matrixy4x4& a, const Matrixy4x4& b);

	/**
	* \brief Multiplies B with A
	*
	* Multiplies 2 matrices together, in the opposite order of Matrixy4x4::Multiply
	*
	* \sa Matrixy4x4::Multiply
	*
	* \param a is the matrix whose transformation is applied first
	* \param b is the matrix whose transformation is applied last
	* \return Returns the transformation that applies A then B
	*/
	inline static Matrixy4x4 MultiplyE(const Matrixy4x4& a, const Matrixy4x4& b) { return Multiply(b, a); }
	inline floaty3 Transform(const floaty3& vec) const { return Transform(vec, *this); }
	inline floaty4 Transform(const floaty4& vec) const { return Transform(vec, *this); }
	static floaty3 Transform(const floaty3& vec, const Matrixy4x4& mat);
	static floaty4 Transform(const floaty4& vec, const Matrixy4x4& mat);
	inline floaty4 TransformNormal(const floaty4& vec) const { floaty3 out = TransformNormal({ vec.x, vec.y, vec.z }); return { out.x, out.y, out.z, 1.f }; }
	inline floaty3 TransformNormal(const floaty3& vec) const { return TransformNormal(vec, *this); }
	static floaty3 TransformNormal(const floaty3& vec, const Matrixy4x4& mat);

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
	static Matrixy4x4 Transpose(const Matrixy4x4& mat);
	static bool Inverted(Matrixy4x4 in, Matrixy4x4& out);
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

	constexpr static inline Matrixy4x4 OrthoProject(float minX, float maxX, float minY, float maxY, float nearZ, float farZ)
	{
		float width = (maxX - minX);
		float height = (maxY - minY);
		float depth = (farZ - nearZ);
		Matrixy4x4 out{ Matrixy4x4::Identity() };
		out.m[0][0] = 2.f / width;	out.m[0][1] = 0;			out.m[0][2] = 0;            out.m[0][3] = 0;
		out.m[1][0] = 0;			out.m[1][1] = 2.f / height;	out.m[1][2] = 0;            out.m[1][3] = 0;
		out.m[2][0] = 0;			out.m[2][1] = 0;			out.m[2][2] = -2.f / depth; out.m[2][3] = 0;
		out.m[3][0] = -(maxX + minX) / width;out.m[3][1] = -(maxY + minY) / height;out.m[3][2] = -(farZ + nearZ) / depth;out.m[3][3] = 1.f;
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

	static inline Matrixy4x4 Scale(float x, float y, float z)
	{
		Matrixy4x4 out;
		out.m11 = x;
		out.m22 = y;
		out.m33 = z;
		out.m44 = 1.f;
		return out;
	}
	static inline Matrixy4x4 Scale(float scale)
	{
		return Scale(scale, scale, scale);
	}


	inline operator Matrixy2x3() const { return { ma[0], ma[1], ma[3], ma[4], ma[5], ma[7] }; }
	operator btTransform() const;
};
#pragma warning(pop)

inline floaty2 operator*(const floaty2& point, const Matrixy2x3& matrix) noexcept
{
	return matrix.TransformPoint(point);
}

inline floaty2 operator*(float scalar, const floaty2& vec)
{
	return { vec.x * scalar, vec.y * scalar };
}
inline floaty2 operator*(const floaty2& vec, float scalar)
{
	return { vec.x * scalar, vec.y * scalar };
}

inline floaty2 operator+(const floaty2& a, const floaty2& b)
{
	return { a.x + b.x, a.y + b.y };
}
inline floaty2 operator-(const floaty2& a)
{
	return { -a.x, -a.y };
}
inline floaty2 operator-(const floaty2& a, const floaty2& b)
{
	return a + (-b);
}

