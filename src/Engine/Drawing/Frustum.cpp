#include "Frustum.h"

template<> bool Voxel::BehindPlane<floaty3>(const Plane &p, const floaty3 &point)
{
	return p.Normal.dot(point) < p.Distance;
}

template<> bool Voxel::BehindPlane<Voxel::Sphere>(const Plane &p, const Sphere &s)
{
	return p.Normal.dot(s.Centre) - p.Distance < -s.Radius;
}

template<> bool Voxel::InsideFrustum<Voxel::Sphere>(const Frustum &f, const Sphere &s)
{
	bool inside = true;
	for (int i = 0; i < 6; ++i)
	{
		if (BehindPlane(f.Planes[i], s))
		{
			inside = false;
			break;
		}
	}
	return inside;
}

template<> bool Voxel::BehindPlane<Voxel::BoxAsPoints>(const Plane &p, const BoxAsPoints &b)
{
	bool inside = true;
	for (int i = 0; i < 8; ++i)
	{
		if (BehindPlane(p, b.Points[i]))
		{
			inside = false;
			break;
		}
	}
	return inside;
}



template<> bool Voxel::InsideFrustum<Voxel::Sphere, Voxel::CameraFrustum>(const CameraFrustum &f, const Sphere &s)
{
	if (f.EncasingSphere.Radius <= 0.f)
		return InsideFrustum<Sphere, Frustum>(f, s);

	if (!SpheresIntersect(f.EncasingSphere, s))
		return false;

	return InsideFrustum<Sphere, Frustum>(f, s);
}

template<> bool Voxel::InsideFrustum<Voxel::BoxAsPoints, Voxel::Frustum>(const Frustum &f, const BoxAsPoints &b)
{
	bool inside = true;
	for (int i = 0; i < 6; ++i)
	{
		if (BehindPlane(f.Planes[i], b))
		{
			inside = false;
			break;
		}
	}
	return inside;
}