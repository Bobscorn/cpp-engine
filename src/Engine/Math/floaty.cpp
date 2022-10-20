#include "floaty.h"

#include "Helpers/VectorHelper.h"

#include <Bullet3Common/b3Vector3.h>
#include <LinearMath/btQuaternion.h>
#include <LinearMath/btVector3.h>
#include <assimp/vector3.h>
#include <SDL_pixels.h>

floaty3::floaty3(const DOUBLE3& a)
	: x(a.x), y(a.y), z(a.z)
{
}

floaty3::floaty3(const b3Vector3& a)
	: x(a.x), y(a.y), z(a.z)
{
}

floaty3::floaty3(const btVector3& a)
	: x(a.x()), y(a.y()), z(a.z())
{
}

floaty3::operator b3Vector3() const
{
	return b3MakeVector3(x, y, z);
}


floaty3::operator btVector3() const
{
	return btVector3(x, y, z);
}

floaty3::operator DOUBLE3() const
{
	return DOUBLE3(x, y, z);
}

floaty3::floaty3(const aiVector3D& a)
	: x(a.x), y(a.y), z(a.z)
{
}

floaty4::floaty4(const btQuaternion& a)
	: x(a.x()), y(a.y()), z(a.z()), w(a.w())
{
}

floaty4::floaty4(const b3Vector4& a)
	: x(a.x), y(a.y), z(a.z), w(a.w)
{
}

floaty4::floaty4(const btVector3& a)
	: x(a.x()), y(a.y()), z(a.z()), w(0.f)
{
}

floaty4::floaty4(const aiVector3D& a)
	: x(a.x), y(a.y), z(a.z), w(0.f)
{
}

floaty4::operator b3Vector4() const
{
	return b3MakeVector4(x, y, z, w);
}

floaty4::operator btVector4() const
{
	return btVector4(x, y, z, w);
}

floaty4::operator btQuaternion() const
{
	return btQuaternion(x, y, z, w);
}

floaty4::floaty4(SDL_Color color) : x(((float)color.r) / 255.f), y(((float)color.g) / 255.f), z(((float)color.b) / 255.f), w(((float)color.a) / 255.f) {}

