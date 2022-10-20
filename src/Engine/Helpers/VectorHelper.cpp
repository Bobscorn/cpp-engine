#include "VectorHelper.h"

#include "Math/floaty.h"

#include "Helpers/DebugHelper.h"

void OutputSizeTest()
{
	DINFO("Library sizeof void*: " + std::to_string(sizeof(void*)));
}


struct Sphere
{
	floaty3 Origin;
	float Radius;
};

struct AABB
{
	floaty3 Min;
	floaty3 Max;
};

void ComputeFrustum(Frustum * out, const Matrixy4x4 & projection)
{
	static constexpr floaty4 HomogenousPoints[6] =
	{
		{ 1.f, 0.f, 1.f, 1.f }, // Right
		{ -1.f, 0.f, 1.f, 1.f }, // Left
		{ 0.f, 1.f, 1.f, 1.f }, // Top
		{ 0.f, -1.f, 1.f, 1.f }, // Bottom
		{ 0.f, 0.f, 0.f, 1.f }, // Near Plane
		{ 0.f, 0.f, 1.f, 1.f }, // Far Plane
	};

	Matrixy4x4 invproj;
	if (!Matrixy4x4::Inverted(projection, invproj))
	{
		DWARNING("Cannot compute frustum with un-inversible projection matrix");
		return;
	}

	floaty4 Points[6];

	for (size_t i = 6; i-- > 0; )
	{
		Points[i] = invproj.Transform(HomogenousPoints[i]);
	}

	out->Origin = { 0.f, 0.f, 0.f };
	out->Orientation = { 0.f, 0.f, 0.f, 1.f };

	Points[0] = Points[0] / Points[0].z;
	Points[1] = Points[1] / Points[1].z;
	Points[2] = Points[2] / Points[2].z;
	Points[3] = Points[3] / Points[3].z;

	out->RightSlope = Points[0].x;
	out->LeftSlope = Points[1].x;
	out->TopSlope = Points[2].y;
	out->BottomSlope = Points[3].y;

	// Near/Far Planes
	Points[4] = Points[4] / Points[4].w;
	Points[5] = Points[5] / Points[5].w;

	out->Near = Points[4].z;
	out->Far = Points[5].z;

	return;
}

int CheckFrustumIntersection(const Sphere *a, const Frustum *b);
int CheckFrustumIntersection(const AABB *a, const Frustum *b);


#ifdef CPP_ENGINE_TESTS

int Helpers_VectorHelper(int argc, const char* argv[])
{
	
}


#endif