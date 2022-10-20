#include "quat4.h"

#include <cmath>

floaty3 quat4::rotate(const floaty3& dir) const
{
	return floaty3(quatRotate((btQuaternion)*this, (btVector3)dir));
}

Vector::inty3 quat4::rotate(const Vector::inty3& dir) const
{
	floaty3 dirF = floaty3((float)dir.x, (float)dir.y, (float)dir.z);
	dirF = rotate(dirF);
	return Vector::inty3((int)std::lroundf(dirF.x), (int)std::lroundf(dirF.y), (int)std::lroundf(dirF.z));
}
