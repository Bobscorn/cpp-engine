#include "matrix.h"

#include "Helpers/VectorHelper.h"
#include "Helpers/SDLHelper.h"
#include "Helpers/MathHelper.h"

#include <LinearMath/btTransform.h>

Matrixy3x3::Matrixy3x3(const btMatrix3x3& other)
	: Matrixy3x3(other[0].getX(), other[0].getY(), other[0].getZ(), other[1].getX(), other[1].getY(), other[1].getZ(), other[2].getX(), other[2].getY(), other[2].getZ())
{
}

Matrixy3x3 Matrixy3x3::operator*(const Matrixy3x3& other)
{
	return (Matrixy4x4::Multiply(Matrixy4x4(*this), Matrixy4x4(other))).As3x3();
}

floaty3 Matrixy3x3::Transform(const floaty3& vec, const Matrixy3x3& mat)
{
	floaty3 out;
	out.x = vec.x * mat.m11 + vec.y * mat.m12 + vec.z * mat.m13;
	out.y = vec.x * mat.m21 + vec.y * mat.m22 + vec.z * mat.m23;
	out.z = vec.x * mat.m31 + vec.y * mat.m32 + vec.z * mat.m33;
	return out;
}

Matrixy3x3 Matrixy3x3::Scale(float x, float y, float z)
{
	return {
		x, 0.f, 0.f,
		0.f, y, 0.f,
		0.f, 0.f, z
	};
}

Matrixy3x3 Matrixy3x3::RotationAxisR(floaty3 axis, float angle)
{
	axis.normalize();
	return RotationAxisNormR(axis, angle);
}

Matrixy3x3 Matrixy3x3::RotationAxisD(floaty3 axis, float angle)
{
	axis.normalize();
	return RotationAxisNormD(axis, angle);
}

// Based on wikipedia's math at https://en.wikipedia.org/wiki/Rotation_matrix#Rotation_matrix_from_axis_and_angle
Matrixy3x3 Matrixy3x3::RotationAxisNormR(floaty3 axis, float angle)
{
	float ca = cosf(angle);
	float sa = sinf(angle);
	float x = axis.x;
	float y = axis.y;
	float z = axis.z;
	float x2 = x * x;
	float y2 = y * y;
	float z2 = z * z;
	return
	{
		ca + x2 * (1.f - ca),			x * y * (1.f - ca) - z * sa,	x * z * (1.f - ca) + y * sa,
		y * x * (1.f - ca) + z * sa,	ca + y2 * (1.f - ca),			y * z * (1.f - ca) - x * sa,
		z * x * (1.f - ca) - y * sa,	z * y * (1.f - ca) + x * sa,	ca + z2 * (1.f - ca)
	};
}

Matrixy3x3 Matrixy3x3::RotationAxisNormD(floaty3 axis, float angle)
{
	return RotationAxisNormR(axis, angle * Math::DegToRadF);
}

Matrixy3x3 Matrixy3x3::RotationR(floaty3 by)
{
	return Matrixy3x3::RotationR(by.x, by.y, by.z);
}

Matrixy3x3 Matrixy3x3::RotationD(floaty3 by)
{
	return Matrixy3x3::RotationD(by.x, by.y, by.z);
}

Matrixy3x3 Matrixy3x3::RotationR(float x, float y, float z)
{
	return Matrixy3x3::RotationX(x) * Matrixy3x3::RotationY(y) * Matrixy3x3::RotationZ(z);
}

Matrixy3x3 Matrixy3x3::RotationD(float x, float y, float z)
{
	return RotationR(x * Math::DegToRadF, y * Math::DegToRadF, z * Math::DegToRadF);
}

Matrixy3x3 Matrixy3x3::RotationX(float x)
{
	return
	{
		1.f, 0.f, 0.f,
		0.f, cosf(x), -sinf(x),
		0.f, sinf(x), cosf(x)
	};
}

Matrixy3x3 Matrixy3x3::RotationY(float y)
{
	return
	{
		cosf(y), 0.f, sinf(y),
		0.f,     1.f, 0.f,
		-sinf(y), 0.f, cosf(y)
	};
}

Matrixy3x3 Matrixy3x3::RotationZ(float z)
{
	return
	{
		cosf(z), -sin(z), 0.f,
		sinf(z), cosf(z), 0.f,
		0.f, 0.f, 1.f
	};
}

Matrixy3x3 Matrixy3x3::ReflectionOrigin(floaty3 along)
{
	auto& a = along;
	float mag = along.magnitude();
	float mag2 = mag * mag;
	Matrixy3x3 out;
	for (unsigned int i = 0; i < 3; ++i)
	{
		for (unsigned int j = 0; j < 3; ++j)
		{
			out.m[i][j] = (i == j ? 1.f : 0.f) - 2 * ((a.m[i] * a.m[j]) / mag2);
		}
	}
	return out;
}

Matrixy3x3 Matrixy3x3::Transpose(const Matrixy3x3& mat)
{
	Matrixy3x3 out;
	out.m11 = mat.m11;
	out.m12 = mat.m21;
	out.m13 = mat.m31;

	out.m21 = mat.m12;
	out.m22 = mat.m22;
	out.m23 = mat.m32;

	out.m31 = mat.m13;
	out.m32 = mat.m23;
	out.m33 = mat.m33;
	return out;
}

PointRect Matrixy2x3::TransformRect(const PointRect& rect) const
{
	floaty2 tmpa, tmpb;
	tmpa = TransformPoint({ rect.left, rect.top });
	tmpb = TransformPoint({ rect.right, rect.bottom });
	return { tmpa.x, tmpa.y, tmpb.x, tmpb.y };
}



Matrixy4x4 Matrixy4x4::Multiply(const Matrixy4x4& a, const Matrixy4x4& b)
{
	Matrixy4x4 out;

	float a00 = a.m[0][0];
	float a01 = a.m[0][1];
	float a02 = a.m[0][2];
	float a03 = a.m[0][3];

	float a10 = a.m[1][0];
	float a11 = a.m[1][1];
	float a12 = a.m[1][2];
	float a13 = a.m[1][3];

	float a20 = a.m[2][0];
	float a21 = a.m[2][1];
	float a22 = a.m[2][2];
	float a23 = a.m[2][3];

	float a30 = a.m[3][0];
	float a31 = a.m[3][1];
	float a32 = a.m[3][2];
	float a33 = a.m[3][3];

	float b00 = b.m[0][0];
	float b01 = b.m[0][1];
	float b02 = b.m[0][2];
	float b03 = b.m[0][3];

	float b10 = b.m[1][0];
	float b11 = b.m[1][1];
	float b12 = b.m[1][2];
	float b13 = b.m[1][3];

	float b20 = b.m[2][0];
	float b21 = b.m[2][1];
	float b22 = b.m[2][2];
	float b23 = b.m[2][3];

	float b30 = b.m[3][0];
	float b31 = b.m[3][1];
	float b32 = b.m[3][2];
	float b33 = b.m[3][3];

	out.m[0][0] = a00 * b00 + a10 * b01 + a20 * b02 + a30 * b03;
	out.m[0][1] = a01 * b00 + a11 * b01 + a21 * b02 + a31 * b03;
	out.m[0][2] = a02 * b00 + a12 * b01 + a22 * b02 + a32 * b03;
	out.m[0][3] = a03 * b00 + a13 * b01 + a23 * b02 + a33 * b03;

	out.m[1][0] = a00 * b10 + a10 * b11 + a20 * b12 + a30 * b13;
	out.m[1][1] = a01 * b10 + a11 * b11 + a21 * b12 + a31 * b13;
	out.m[1][2] = a02 * b10 + a12 * b11 + a22 * b12 + a32 * b13;
	out.m[1][3] = a03 * b10 + a13 * b11 + a23 * b12 + a33 * b13;

	out.m[2][0] = a00 * b20 + a10 * b21 + a20 * b22 + a30 * b23;
	out.m[2][1] = a01 * b20 + a11 * b21 + a21 * b22 + a31 * b23;
	out.m[2][2] = a02 * b20 + a12 * b21 + a22 * b22 + a32 * b23;
	out.m[2][3] = a03 * b20 + a13 * b21 + a23 * b22 + a33 * b23;

	out.m[3][0] = a00 * b30 + a10 * b31 + a20 * b32 + a30 * b33;
	out.m[3][1] = a01 * b30 + a11 * b31 + a21 * b32 + a31 * b33;
	out.m[3][2] = a02 * b30 + a12 * b31 + a22 * b32 + a32 * b33;
	out.m[3][3] = a03 * b30 + a13 * b31 + a23 * b32 + a33 * b33;

	return out;
}

floaty3 Matrixy4x4::Transform(const floaty3& vec, const Matrixy4x4& mat)
{
	floaty3 out;
	out.x = vec.x * mat.m11 + vec.y * mat.m12 + vec.z * mat.m13 + mat.dx;
	out.y = vec.x * mat.m21 + vec.y * mat.m22 + vec.z * mat.m23 + mat.dy;
	out.z = vec.x * mat.m31 + vec.y * mat.m32 + vec.z * mat.m33 + mat.dz;
	return out;
}

floaty4 Matrixy4x4::Transform(const floaty4& vec, const Matrixy4x4& mat)
{
	floaty4 out;
	out.x = vec.x * mat.m11 + vec.y * mat.m12 + vec.z * mat.m13 + vec.w * mat.dx;
	out.y = vec.x * mat.m21 + vec.y * mat.m22 + vec.z * mat.m23 + vec.w * mat.dy;
	out.z = vec.x * mat.m31 + vec.y * mat.m32 + vec.z * mat.m33 + vec.w * mat.dz;
	out.w = vec.x * mat.m41 + vec.y * mat.m42 + vec.z * mat.m43 + vec.w * mat.m44;
	return out;
}

floaty3 Matrixy4x4::TransformNormal(const floaty3& vec, const Matrixy4x4& mat)
{
	floaty3 out;
	out.x = vec.x * mat.m11 + vec.y * mat.m12 + vec.z * mat.m13;
	out.y = vec.x * mat.m21 + vec.y * mat.m22 + vec.z * mat.m23;
	out.z = vec.x * mat.m31 + vec.y * mat.m32 + vec.z * mat.m33;
	return out;
}

Matrixy4x4 Matrixy4x4::RotationAxisR(floaty3 axis, float angle)
{
	axis.safenorm();
	if (axis.mag2() == 0.f)
		return Matrixy4x4::Identity();
	return Matrixy4x4(Matrixy3x3::RotationAxisNormR(axis, angle));
}

Matrixy4x4 Matrixy4x4::RotationAxisD(floaty3 axis, float angle)
{
	return Matrixy4x4(Matrixy3x3::RotationAxisD(axis, angle));
}

Matrixy4x4 Matrixy4x4::Reflection(floaty3 along, floaty3 origin)
{
	Matrixy4x4 out = Matrixy4x4::Translate(-origin.x, -origin.y, -origin.z);
	out = Matrixy4x4::MultiplyE(out, Matrixy4x4(Matrixy3x3::ReflectionOrigin(along)));
	out = Matrixy4x4::MultiplyE(out, Matrixy4x4::Translate(origin.x, origin.y, origin.z));
	return out;
}

Matrixy4x4 Matrixy4x4::Transpose(const Matrixy4x4& mat)
{
	Matrixy4x4 out;
	out._11 = mat._11;
	out._12 = mat._21;
	out._13 = mat._31;
	out._14 = mat._41;

	out._21 = mat._12;
	out._22 = mat._22;
	out._23 = mat._32;
	out._24 = mat._42;

	out._31 = mat._13;
	out._32 = mat._23;
	out._33 = mat._33;
	out._34 = mat._43;

	out._41 = mat._14;
	out._42 = mat._24;
	out._43 = mat._34;
	out._44 = mat._44;
	return out;
}

bool Matrixy4x4::Inverted(Matrixy4x4 in, Matrixy4x4& out)
{
	bool caninvert = in.Invert();
	if (!caninvert)
		return false;

	out = in;
	return true;
}

bool Matrixy4x4::Invert()
{
	float inv[16], det;
	int i;

	inv[0] = ma[5] * ma[10] * ma[15] -
		ma[5] * ma[11] * ma[14] -
		ma[9] * ma[6] * ma[15] +
		ma[9] * ma[7] * ma[14] +
		ma[13] * ma[6] * ma[11] -
		ma[13] * ma[7] * ma[10];

	inv[4] = -ma[4] * ma[10] * ma[15] +
		ma[4] * ma[11] * ma[14] +
		ma[8] * ma[6] * ma[15] -
		ma[8] * ma[7] * ma[14] -
		ma[12] * ma[6] * ma[11] +
		ma[12] * ma[7] * ma[10];

	inv[8] = ma[4] * ma[9] * ma[15] -
		ma[4] * ma[11] * ma[13] -
		ma[8] * ma[5] * ma[15] +
		ma[8] * ma[7] * ma[13] +
		ma[12] * ma[5] * ma[11] -
		ma[12] * ma[7] * ma[9];

	inv[12] = -ma[4] * ma[9] * ma[14] +
		ma[4] * ma[10] * ma[13] +
		ma[8] * ma[5] * ma[14] -
		ma[8] * ma[6] * ma[13] -
		ma[12] * ma[5] * ma[10] +
		ma[12] * ma[6] * ma[9];

	inv[1] = -ma[1] * ma[10] * ma[15] +
		ma[1] * ma[11] * ma[14] +
		ma[9] * ma[2] * ma[15] -
		ma[9] * ma[3] * ma[14] -
		ma[13] * ma[2] * ma[11] +
		ma[13] * ma[3] * ma[10];

	inv[5] = ma[0] * ma[10] * ma[15] -
		ma[0] * ma[11] * ma[14] -
		ma[8] * ma[2] * ma[15] +
		ma[8] * ma[3] * ma[14] +
		ma[12] * ma[2] * ma[11] -
		ma[12] * ma[3] * ma[10];

	inv[9] = -ma[0] * ma[9] * ma[15] +
		ma[0] * ma[11] * ma[13] +
		ma[8] * ma[1] * ma[15] -
		ma[8] * ma[3] * ma[13] -
		ma[12] * ma[1] * ma[11] +
		ma[12] * ma[3] * ma[9];

	inv[13] = ma[0] * ma[9] * ma[14] -
		ma[0] * ma[10] * ma[13] -
		ma[8] * ma[1] * ma[14] +
		ma[8] * ma[2] * ma[13] +
		ma[12] * ma[1] * ma[10] -
		ma[12] * ma[2] * ma[9];

	inv[2] = ma[1] * ma[6] * ma[15] -
		ma[1] * ma[7] * ma[14] -
		ma[5] * ma[2] * ma[15] +
		ma[5] * ma[3] * ma[14] +
		ma[13] * ma[2] * ma[7] -
		ma[13] * ma[3] * ma[6];

	inv[6] = -ma[0] * ma[6] * ma[15] +
		ma[0] * ma[7] * ma[14] +
		ma[4] * ma[2] * ma[15] -
		ma[4] * ma[3] * ma[14] -
		ma[12] * ma[2] * ma[7] +
		ma[12] * ma[3] * ma[6];

	inv[10] = ma[0] * ma[5] * ma[15] -
		ma[0] * ma[7] * ma[13] -
		ma[4] * ma[1] * ma[15] +
		ma[4] * ma[3] * ma[13] +
		ma[12] * ma[1] * ma[7] -
		ma[12] * ma[3] * ma[5];

	inv[14] = -ma[0] * ma[5] * ma[14] +
		ma[0] * ma[6] * ma[13] +
		ma[4] * ma[1] * ma[14] -
		ma[4] * ma[2] * ma[13] -
		ma[12] * ma[1] * ma[6] +
		ma[12] * ma[2] * ma[5];

	inv[3] = -ma[1] * ma[6] * ma[11] +
		ma[1] * ma[7] * ma[10] +
		ma[5] * ma[2] * ma[11] -
		ma[5] * ma[3] * ma[10] -
		ma[9] * ma[2] * ma[7] +
		ma[9] * ma[3] * ma[6];

	inv[7] = ma[0] * ma[6] * ma[11] -
		ma[0] * ma[7] * ma[10] -
		ma[4] * ma[2] * ma[11] +
		ma[4] * ma[3] * ma[10] +
		ma[8] * ma[2] * ma[7] -
		ma[8] * ma[3] * ma[6];

	inv[11] = -ma[0] * ma[5] * ma[11] +
		ma[0] * ma[7] * ma[9] +
		ma[4] * ma[1] * ma[11] -
		ma[4] * ma[3] * ma[9] -
		ma[8] * ma[1] * ma[7] +
		ma[8] * ma[3] * ma[5];

	inv[15] = ma[0] * ma[5] * ma[10] -
		ma[0] * ma[6] * ma[9] -
		ma[4] * ma[1] * ma[10] +
		ma[4] * ma[2] * ma[9] +
		ma[8] * ma[1] * ma[6] -
		ma[8] * ma[2] * ma[5];

	det = ma[0] * inv[0] + ma[1] * inv[4] + ma[2] * inv[8] + ma[3] * inv[12];

	if (det == 0)
		return false;

	det = 1.f / det;

	for (i = 0; i < 16; i++)
		ma[i] = inv[i] * det;

	return true;
}

Matrixy4x4 Matrixy4x4::LookAt(floaty3 eye, floaty3 target, floaty3 up)
{
	floaty3 F = { target.x - eye.x, target.y - eye.y, target.z - eye.z };
	F.normalize();
	up.normalize();

	floaty3 s = F.cross(up);
	floaty3 u = floaty3::Normalized(s).cross(F);

	return {
		+s.x, +s.y, +s.z, -eye.dot(s),
		+u.x, +u.y, +u.z, -eye.dot(u),
		-F.x, -F.y, -F.z, eye.dot(F),
		+0.f, +0.f, +0.f, 1.f
	};
}

Matrixy4x4 Matrixy4x4::PerspectiveFovD(float fov, float aspect, float nearZ, float farZ)
{
	float yScale = 1.f / tanf(Math::DegToRadF * fov / 2);
	float xScale = yScale / aspect;
	float nearmfar = nearZ - farZ;
	return {
		xScale, 0.f, 0.f, 0.f,
		0.f, yScale, 0.f, 0.f,
		0.f, 0.f, (farZ + nearZ) / nearmfar, -1.f,
		0.f, 0.f, 2.f * farZ * nearZ / nearmfar, 0.f
	};
}

Matrixy4x4 Matrixy4x4::PerspectiveFovR(float fov, float aspect, float nearZ, float farZ)
{
	float yScale = 1.f / tanf(fov / 2);
	float xScale = yScale / aspect;
	float nearmfar = nearZ - farZ;
	return {
		xScale, 0.f, 0.f, 0.f,
		0.f, yScale, 0.f, 0.f,
		0.f, 0.f, (farZ + nearZ) / nearmfar, 2.f * farZ * nearZ / nearmfar,
		0.f, 0.f, -1.f, 0.f
	};
}

Matrixy4x4::operator btTransform() const
{
	btTransform out; 
	out.setFromOpenGLMatrix(ma); 
	return out;
}


#ifdef CPP_ENGINE_TESTS

#include <gtest/gtest.h>
#include <gmock/gmock.h>

using namespace testing;

TEST(MatrixTests, DefaultConstructorIdentity)
{
	Matrixy4x4 mat{};
	ASSERT_EQ(mat.m11, 1.f); ASSERT_EQ(mat.m21, 0.f); ASSERT_EQ(mat.m31, 0.f); ASSERT_EQ(mat.m41, 0.f);
	ASSERT_EQ(mat.m12, 0.f); ASSERT_EQ(mat.m22, 1.f); ASSERT_EQ(mat.m32, 0.f); ASSERT_EQ(mat.m42, 0.f);
	ASSERT_EQ(mat.m13, 0.f); ASSERT_EQ(mat.m23, 0.f); ASSERT_EQ(mat.m33, 1.f); ASSERT_EQ(mat.m43, 0.f);
	ASSERT_EQ(mat.dx, 0.f);  ASSERT_EQ(mat.dy, 0.f);  ASSERT_EQ(mat.dz, 0.f);  ASSERT_EQ(mat.m44, 1.f);
	ASSERT_EQ(mat, Matrixy4x4::Identity());
}

TEST(MatrixTests, IdentityTest)
{
	Matrixy4x4 mat = Matrixy4x4::Identity();
	Matrixy4x4 mat2 = Matrixy4x4::Identity();
	ASSERT_EQ(mat, mat2);
	ASSERT_EQ(mat, Matrixy4x4::Identity());

	ASSERT_EQ(mat.m11, 1.f); ASSERT_EQ(mat.m21, 0.f); ASSERT_EQ(mat.m31, 0.f); ASSERT_EQ(mat.m41, 0.f);
	ASSERT_EQ(mat.m12, 0.f); ASSERT_EQ(mat.m22, 1.f); ASSERT_EQ(mat.m32, 0.f); ASSERT_EQ(mat.m42, 0.f);
	ASSERT_EQ(mat.m13, 0.f); ASSERT_EQ(mat.m23, 0.f); ASSERT_EQ(mat.m33, 1.f); ASSERT_EQ(mat.m43, 0.f);
	ASSERT_EQ(mat.dx, 0.f);  ASSERT_EQ(mat.dy, 0.f);  ASSERT_EQ(mat.dz, 0.f);  ASSERT_EQ(mat.m44, 1.f);
}

TEST(MatrixTests, ValidityOfUnionTest)
{
	Matrixy4x4 mat{};
	std::memset(mat.ma, 0, sizeof(mat));
	ASSERT_THAT(0.f, AllOf(mat.m11, mat._11, mat.m[0][0], mat.ma[0]));
	ASSERT_THAT(0.f, AllOf(mat.m21, mat._21, mat.m[0][1], mat.ma[1]));
	ASSERT_THAT(0.f, AllOf(mat.m31, mat._31, mat.m[0][2], mat.ma[2]));
	ASSERT_THAT(0.f, AllOf(mat.m41, mat._41, mat.m[0][3], mat.ma[3]));

	ASSERT_THAT(0.f, AllOf(mat.m12, mat._12, mat.m[1][0], mat.ma[4 * 1 + 0]));
	ASSERT_THAT(0.f, AllOf(mat.m22, mat._22, mat.m[1][1], mat.ma[4 * 1 + 1]));
	ASSERT_THAT(0.f, AllOf(mat.m32, mat._32, mat.m[1][2], mat.ma[4 * 1 + 2]));
	ASSERT_THAT(0.f, AllOf(mat.m42, mat._42, mat.m[1][3], mat.ma[4 * 1 + 3]));

	ASSERT_THAT(0.f, AllOf(mat.m13, mat._13, mat.m[2][0], mat.ma[0 + 2 * 4]));
	ASSERT_THAT(0.f, AllOf(mat.m23, mat._23, mat.m[2][1], mat.ma[1 + 2 * 4]));
	ASSERT_THAT(0.f, AllOf(mat.m33, mat._33, mat.m[2][2], mat.ma[2 + 2 * 4]));
	ASSERT_THAT(0.f, AllOf(mat.m43, mat._43, mat.m[2][3], mat.ma[3 + 2 * 4]));

	ASSERT_THAT(0.f, AllOf(mat.dx,  mat._14, mat.m[3][0], mat.ma[0 + 3 * 4]));
	ASSERT_THAT(0.f, AllOf(mat.dy,  mat._24, mat.m[3][1], mat.ma[1 + 3 * 4]));
	ASSERT_THAT(0.f, AllOf(mat.dz,  mat._34, mat.m[3][2], mat.ma[2 + 3 * 4]));
	ASSERT_THAT(0.f, AllOf(mat.m44, mat._44, mat.m[3][3], mat.ma[3 + 3 * 4]));

	mat = Matrixy4x4(1.f, 2.f, 3.f, 4.f, 5.f, 6.f, 7.f, 8.f, 9.f, 10.f, 11.f, 12.f, 13.f, 14.f, 15.f, 16.f);
	ASSERT_THAT(1.f, AllOf(mat.m11, mat._11, mat.m[0][0], mat.ma[0]));
	ASSERT_THAT(2.f, AllOf(mat.m21, mat._21, mat.m[0][1], mat.ma[1]));
	ASSERT_THAT(3.f, AllOf(mat.m31, mat._31, mat.m[0][2], mat.ma[2]));
	ASSERT_THAT(4.f, AllOf(mat.m41, mat._41, mat.m[0][3], mat.ma[3]));

	ASSERT_THAT(5.f, AllOf(mat.m12, mat._12, mat.m[1][0], mat.ma[4 * 1 + 0]));
	ASSERT_THAT(6.f, AllOf(mat.m22, mat._22, mat.m[1][1], mat.ma[4 * 1 + 1]));
	ASSERT_THAT(7.f, AllOf(mat.m32, mat._32, mat.m[1][2], mat.ma[4 * 1 + 2]));
	ASSERT_THAT(8.f, AllOf(mat.m42, mat._42, mat.m[1][3], mat.ma[4 * 1 + 3]));

	ASSERT_THAT(9.f,  AllOf(mat.m13, mat._13, mat.m[2][0], mat.ma[0 + 2 * 4]));
	ASSERT_THAT(10.f, AllOf(mat.m23, mat._23, mat.m[2][1], mat.ma[1 + 2 * 4]));
	ASSERT_THAT(11.f, AllOf(mat.m33, mat._33, mat.m[2][2], mat.ma[2 + 2 * 4]));
	ASSERT_THAT(12.f, AllOf(mat.m43, mat._43, mat.m[2][3], mat.ma[3 + 2 * 4]));

	ASSERT_THAT(13.f, AllOf(mat.dx,  mat._14, mat.m[3][0], mat.ma[0 + 3 * 4]));
	ASSERT_THAT(14.f, AllOf(mat.dy,  mat._24, mat.m[3][1], mat.ma[1 + 3 * 4]));
	ASSERT_THAT(15.f, AllOf(mat.dz,  mat._34, mat.m[3][2], mat.ma[2 + 3 * 4]));
	ASSERT_THAT(16.f, AllOf(mat.m44, mat._44, mat.m[3][3], mat.ma[3 + 3 * 4]));

	Matrixy4x4 mat2 = mat;
	mat = mat2;
	ASSERT_THAT(1.f, AllOf(mat.m11, mat._11, mat.m[0][0], mat.ma[0]));
	ASSERT_THAT(2.f, AllOf(mat.m21, mat._21, mat.m[0][1], mat.ma[1]));
	ASSERT_THAT(3.f, AllOf(mat.m31, mat._31, mat.m[0][2], mat.ma[2]));
	ASSERT_THAT(4.f, AllOf(mat.m41, mat._41, mat.m[0][3], mat.ma[3]));

	ASSERT_THAT(5.f, AllOf(mat.m12, mat._12, mat.m[1][0], mat.ma[4 * 1 + 0]));
	ASSERT_THAT(6.f, AllOf(mat.m22, mat._22, mat.m[1][1], mat.ma[4 * 1 + 1]));
	ASSERT_THAT(7.f, AllOf(mat.m32, mat._32, mat.m[1][2], mat.ma[4 * 1 + 2]));
	ASSERT_THAT(8.f, AllOf(mat.m42, mat._42, mat.m[1][3], mat.ma[4 * 1 + 3]));

	ASSERT_THAT(9.f,  AllOf(mat.m13, mat._13, mat.m[2][0], mat.ma[0 + 2 * 4]));
	ASSERT_THAT(10.f, AllOf(mat.m23, mat._23, mat.m[2][1], mat.ma[1 + 2 * 4]));
	ASSERT_THAT(11.f, AllOf(mat.m33, mat._33, mat.m[2][2], mat.ma[2 + 2 * 4]));
	ASSERT_THAT(12.f, AllOf(mat.m43, mat._43, mat.m[2][3], mat.ma[3 + 2 * 4]));

	ASSERT_THAT(13.f, AllOf(mat.dx,  mat._14, mat.m[3][0], mat.ma[0 + 3 * 4]));
	ASSERT_THAT(14.f, AllOf(mat.dy,  mat._24, mat.m[3][1], mat.ma[1 + 3 * 4]));
	ASSERT_THAT(15.f, AllOf(mat.dz,  mat._34, mat.m[3][2], mat.ma[2 + 3 * 4]));
	ASSERT_THAT(16.f, AllOf(mat.m44, mat._44, mat.m[3][3], mat.ma[3 + 3 * 4]));

	mat2 = mat;
	mat = std::move(mat2);
	ASSERT_THAT(1.f, AllOf(mat.m11, mat._11, mat.m[0][0], mat.ma[0]));
	ASSERT_THAT(2.f, AllOf(mat.m21, mat._21, mat.m[0][1], mat.ma[1]));
	ASSERT_THAT(3.f, AllOf(mat.m31, mat._31, mat.m[0][2], mat.ma[2]));
	ASSERT_THAT(4.f, AllOf(mat.m41, mat._41, mat.m[0][3], mat.ma[3]));

	ASSERT_THAT(5.f, AllOf(mat.m12, mat._12, mat.m[1][0], mat.ma[4 * 1 + 0]));
	ASSERT_THAT(6.f, AllOf(mat.m22, mat._22, mat.m[1][1], mat.ma[4 * 1 + 1]));
	ASSERT_THAT(7.f, AllOf(mat.m32, mat._32, mat.m[1][2], mat.ma[4 * 1 + 2]));
	ASSERT_THAT(8.f, AllOf(mat.m42, mat._42, mat.m[1][3], mat.ma[4 * 1 + 3]));

	ASSERT_THAT(9.f,  AllOf(mat.m13, mat._13, mat.m[2][0], mat.ma[0 + 2 * 4]));
	ASSERT_THAT(10.f, AllOf(mat.m23, mat._23, mat.m[2][1], mat.ma[1 + 2 * 4]));
	ASSERT_THAT(11.f, AllOf(mat.m33, mat._33, mat.m[2][2], mat.ma[2 + 2 * 4]));
	ASSERT_THAT(12.f, AllOf(mat.m43, mat._43, mat.m[2][3], mat.ma[3 + 2 * 4]));

	ASSERT_THAT(13.f, AllOf(mat.dx,  mat._14, mat.m[3][0], mat.ma[0 + 3 * 4]));
	ASSERT_THAT(14.f, AllOf(mat.dy,  mat._24, mat.m[3][1], mat.ma[1 + 3 * 4]));
	ASSERT_THAT(15.f, AllOf(mat.dz,  mat._34, mat.m[3][2], mat.ma[2 + 3 * 4]));
	ASSERT_THAT(16.f, AllOf(mat.m44, mat._44, mat.m[3][3], mat.ma[3 + 3 * 4]));
}

bool ApproximatelyEqual(float a, float b)
{
	static constexpr float e = 0.0001f;

	return a > b - e && a < b + e;
}

TEST(MatrixTests, RotationTest)
{
	static constexpr float e = 0.0001f;
	Matrixy4x4 mat = Matrixy4x4::RotationAxisD(floaty3{ 0.f, 1.f, 0.f }, 90.f);
	floaty3 vec = floaty3{ 1.f, 0.f, 0.f };
	ASSERT_FLOAT_EQ(vec.x, 1.f);
	ASSERT_FLOAT_EQ(vec.y, 0.f);
	ASSERT_FLOAT_EQ(vec.z, 0.f);
	vec = mat.TransformNormal(vec);

	EXPECT_THAT(vec.x, FloatNear(0.f, e));
	EXPECT_THAT(vec.y, FloatNear(0.f, e));
	EXPECT_THAT(vec.z, FloatNear(1.f, e));

	vec = mat.TransformNormal(vec);

	EXPECT_THAT(vec.x, FloatNear(-1.f, e));
	EXPECT_THAT(vec.y, FloatNear(0.f, e));
	EXPECT_THAT(vec.z, FloatNear(0.f, e));

	mat = Matrixy4x4::RotationAxisD(floaty3{ 1.f, 0.f, 0.f }, 90.f);
	vec = mat.TransformNormal(floaty3{ 0.f, 1.f, 0.f });

	EXPECT_THAT(vec.x, FloatNear(0.f, e));
	EXPECT_THAT(vec.y, FloatNear(0.f, e));
	EXPECT_THAT(vec.z, FloatNear(-1.f, e));

	vec = mat.TransformNormal(vec);

	EXPECT_THAT(vec.x, FloatNear(0.f, e));
	EXPECT_THAT(vec.y, FloatNear(-1.f, e));
	EXPECT_THAT(vec.z, FloatNear(0.f, e));

}

TEST(MatrixTests, ReflectionTest)
{
	Matrixy4x4 mat = Matrixy4x4::Reflection(floaty3{ 0.f, 1.f, 0.f }, floaty3{ 0.f, 0.f, 0.f });

	floaty3 vec = floaty3{ 1.f / sqrtf(2.f), 1.f / sqrtf(2.f), 0.f };
	EXPECT_TRUE(ApproximatelyEqual(vec.x, 1.f / sqrtf(2.f)));
	EXPECT_TRUE(ApproximatelyEqual(vec.y, 1.f / sqrtf(2.f)));
	EXPECT_TRUE(ApproximatelyEqual(vec.z, 0.f));

	vec = mat.TransformNormal(vec);

	EXPECT_TRUE(ApproximatelyEqual(vec.x, 1.f / sqrtf(2.f)));
	EXPECT_TRUE(ApproximatelyEqual(vec.y, -(1.f / sqrtf(2.f))));
	EXPECT_TRUE(ApproximatelyEqual(vec.z, 0.f));
}

#endif // CPP_ENGINE_TESTS

