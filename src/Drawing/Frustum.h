#pragma once

#include "Helpers/VectorHelper.h"

namespace Voxel
{

	struct Plane
	{
		floaty3 Normal;
		float Distance;
	};

	struct Sphere
	{
		floaty3 Centre;
		float Radius;
	};

	struct Frustum
	{
		// Left, Top, Right, Bottom, Near, Far
		Plane Planes[6];
	};

	struct CameraFrustum : Frustum
	{
		Sphere EncasingSphere;
	};

	struct BoxAsPoints
	{
		floaty3 Points[8];
	};

	struct AxisAlignedBox
	{
		floaty3 Centre;
		floaty3 Extents;

		inline operator BoxAsPoints() const
		{
			return { Centre + Extents, Centre + floaty3{ -Extents.x, Extents.y, Extents.z }, Centre + floaty3{ Extents.x, -Extents.y, Extents.z }, Centre + floaty3{ Extents.x, Extents.y, -Extents.z }, Centre + floaty3{ -Extents.x, -Extents.y, Extents.z }, Centre + floaty3{ -Extents.x, Extents.y, -Extents.z }, Centre + floaty3{ Extents.x, -Extents.y, -Extents.z }, Centre + -Extents };
		}
	};

	template<class Type> bool BehindPlane(const Plane &f, const Type &t);

	template<class Type, class FrustumType = Frustum> bool InsideFrustum(const FrustumType &f, const Type &t);

	constexpr bool SpheresIntersect(const Sphere &a, const Sphere &b)
	{
		return (a.Centre - b.Centre).mag2() < (a.Radius + b.Radius) * (a.Radius + b.Radius);
	}
}