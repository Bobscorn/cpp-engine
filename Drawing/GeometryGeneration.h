#pragma once

#include "Graphics3D.h"

#include <vector>

namespace GeoGen
{
	struct MeshData
	{
		std::vector<FullVertex> Vertices;
		std::vector<unsigned int> Indices;
	};

	class GeometryGenerator
	{
	public:
		static MeshData CreateBox(float width, float height, float depth);
		static MeshData CreateSphere(float radius, UINT sliceCount, UINT stackCount);
		static MeshData CreateGeosphere(float radius, UINT numSubdivisions);
		static MeshData CreateCylinder(float bottomRadius, float topRadius, float height, UINT sliceCount, UINT stackCount);
		static MeshData CreateGrid(float width, float depth, UINT m, UINT n);

		//static Geometry::Shape CreateBoxS(float width, float height, float depth, Geometry::Material = Geometry::Material());
		//static Geometry::Shape CreateSphereS(float radius, UINT sliceCount, UINT stackCount, Geometry::Material = Geometry::Material());
		//static Geometry::Shape CreateGeosphereS(float radius, UINT numSubdivisions, Geometry::Material = Geometry::Material());
		//static Geometry::Shape CreateCylinderS(float bottomRadius, float topRadius, float height, UINT sliceCount, UINT stackCount, Geometry::Material = Geometry::Material());
		//static Geometry::Shape CreateGridS(float width, float depth, UINT m, UINT n, Geometry::Material = Geometry::Material());

		//MeshData CreateFullscreenQuad();

	private:

		static void Subdivide(MeshData& meshData);
		static void BuildCylinderTopCap(float bottomRadius, float topRadius, float height, UINT sliceCount, UINT stackCount, MeshData& meshData);
		static void BuildCylinderBottomCap(float bottomRadius, float topRadius, float height, UINT sliceCount, UINT stackCount, MeshData& meshData);



	};

	float AngleFromXY(float x, float y);

};