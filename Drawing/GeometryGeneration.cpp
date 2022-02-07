//#include "Thing.h"
#include "GeometryGeneration.h"

#include <cmath>

#define M_PIF 3.1415926535897982384626433f

GeoGen::MeshData GeoGen::GeometryGenerator::CreateBox(float width, float height, float depth)
{
	MeshData meshData;

	//
	// Create the vertices.
	//

	FullVertex v[24];

	float w2 = 0.5f * width;
	float h2 = 0.5f * height;
	float d2 = 0.5f * depth;

	// Fill in the front face FullVertex data.
	v[0] = FullVertex{ { -w2, -h2, -d2 }, { 1.0f, 0.0f, 0.0f }, { 0.f, -1.f, 0.f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 1.0f } };
	v[1] = FullVertex{ { -w2, +h2, -d2 }, { 1.0f, 0.0f, 0.0f }, { 0.f, -1.f, 0.f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f } };
	v[2] = FullVertex{ { +w2, +h2, -d2 }, { 1.0f, 0.0f, 0.0f }, { 0.f, -1.f, 0.f }, { 0.0f, 0.0f, 1.0f }, { 1.0f, 0.0f } };
	v[3] = FullVertex{ { +w2, -h2, -d2 }, { 1.0f, 0.0f, 0.0f }, { 0.f, -1.f, 0.f }, { 0.0f, 0.0f, 1.0f }, { 1.0f, 1.0f } };

	// Fill in the back face FullVertex data.
	v[4] = FullVertex{ { -w2, -h2, +d2, }, { -1.0f, 0.0f, 0.0f }, { 0.f, -1.f, 0.f }, { 0.0f, 0.0f, -1.0f }, { 1.0f, 1.0f } };
	v[5] = FullVertex{ { +w2, -h2, +d2, }, { -1.0f, 0.0f, 0.0f }, { 0.f, -1.f, 0.f }, { 0.0f, 0.0f, -1.0f }, { 0.0f, 1.0f } };
	v[6] = FullVertex{ { +w2, +h2, +d2, }, { -1.0f, 0.0f, 0.0f }, { 0.f, -1.f, 0.f }, { 0.0f, 0.0f, -1.0f }, { 0.0f, 0.0f } };
	v[7] = FullVertex{ { -w2, +h2, +d2, }, { -1.0f, 0.0f, 0.0f }, { 0.f, -1.f, 0.f }, { 0.0f, 0.0f, -1.0f }, { 1.0f, 0.0f } };

	// Fill in the top face FullVertex data.
	v[8] = FullVertex{ { -w2, +h2, -d2, }, { 1.0f, 0.0f, 0.0f }, { 0.f, 0.f, -1.f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 1.0f } };
	v[9] = FullVertex{ { -w2, +h2, +d2, }, { 1.0f, 0.0f, 0.0f }, { 0.f, 0.f, -1.f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f } };
	v[10] = FullVertex{ { +w2, +h2, +d2, }, { 1.0f, 0.0f, 0.0f }, { 0.f, 0.f, -1.f }, { 0.0f, 1.0f, 0.0f }, { 1.0f, 0.0f } };
	v[11] = FullVertex{ { +w2, +h2, -d2, }, { 1.0f, 0.0f, 0.0f }, { 0.f, 0.f, -1.f }, { 0.0f, 1.0f, 0.0f }, { 1.0f, 1.0f } };

	// Fill in the bottom face FullVertex data.
	v[12] = FullVertex{ { -w2, -h2, -d2, }, { -1.0f, 0.0f, 0.0f }, { 0.f, 0.f, -1.f }, { 0.0f, -1.0f, 0.0f }, { 1.0f, 1.0f } };
	v[13] = FullVertex{ { +w2, -h2, -d2, }, { -1.0f, 0.0f, 0.0f }, { 0.f, 0.f, -1.f }, { 0.0f, -1.0f, 0.0f }, { 0.0f, 1.0f } };
	v[14] = FullVertex{ { +w2, -h2, +d2, }, { -1.0f, 0.0f, 0.0f }, { 0.f, 0.f, -1.f }, { 0.0f, -1.0f, 0.0f }, { 0.0f, 0.0f } };
	v[15] = FullVertex{ { -w2, -h2, +d2, }, { -1.0f, 0.0f, 0.0f }, { 0.f, 0.f, -1.f }, { 0.0f, -1.0f, 0.0f }, { 1.0f, 0.0f } };

	// Fill in the left face FullVertex data.
	v[16] = FullVertex{ { -w2, -h2, +d2, }, { 0.0f, 0.0f, -1.0f }, { 0.f, 0.f, -1.f }, { -1.0f, 0.0f, 0.0f }, { 0.0f, 1.0f } };
	v[17] = FullVertex{ { -w2, +h2, +d2, }, { 0.0f, 0.0f, -1.0f }, { 0.f, 0.f, -1.f }, { -1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f } }; 
	v[18] = FullVertex{ { -w2, +h2, -d2, }, { 0.0f, 0.0f, -1.0f }, { 0.f, 0.f, -1.f }, { -1.0f, 0.0f, 0.0f }, { 1.0f, 0.0f } };
	v[19] = FullVertex{ { -w2, -h2, -d2, }, { 0.0f, 0.0f, -1.0f }, { 0.f, 0.f, -1.f }, { -1.0f, 0.0f, 0.0f }, { 1.0f, 1.0f } };

	// Fill in the right face FullVertex data.
	v[20] = FullVertex{ { +w2, -h2, -d2, }, { 0.0f, 0.0f, 1.0f }, { 0.f, 0.f, -1.f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 1.0f } };
	v[21] = FullVertex{ { +w2, +h2, -d2, }, { 0.0f, 0.0f, 1.0f }, { 0.f, 0.f, -1.f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f } };
	v[22] = FullVertex{ { +w2, +h2, +d2, }, { 0.0f, 0.0f, 1.0f }, { 0.f, 0.f, -1.f }, { 1.0f, 0.0f, 0.0f }, { 1.0f, 0.0f } };
	v[23] = FullVertex{ { +w2, -h2, +d2, }, { 0.0f, 0.0f, 1.0f }, { 0.f, 0.f, -1.f }, { 1.0f, 0.0f, 0.0f }, { 1.0f, 1.0f } };

	meshData.Vertices.assign(&v[0], &v[24]);

	//
	// Create the indices.
	//

	UINT i[36];

	// Fill in the front face index data
	i[0] = 0; i[1] = 1; i[2] = 2;
	i[3] = 0; i[4] = 2; i[5] = 3;

	// Fill in the back face index data
	i[6] = 4; i[7] = 5; i[8] = 6;
	i[9] = 4; i[10] = 6; i[11] = 7;

	// Fill in the top face index data
	i[12] = 8; i[13] = 9; i[14] = 10;
	i[15] = 8; i[16] = 10; i[17] = 11;

	// Fill in the bottom face index data
	i[18] = 12; i[19] = 13; i[20] = 14;
	i[21] = 12; i[22] = 14; i[23] = 15;

	// Fill in the left face index data
	i[24] = 16; i[25] = 17; i[26] = 18;
	i[27] = 16; i[28] = 18; i[29] = 19;

	// Fill in the right face index data
	i[30] = 20; i[31] = 21; i[32] = 22;
	i[33] = 20; i[34] = 22; i[35] = 23;

	meshData.Indices.assign(&i[0], &i[36]);

	return meshData;
}

GeoGen::MeshData GeoGen::GeometryGenerator::CreateSphere(float radius, UINT sliceCount, UINT stackCount)
{
	MeshData meshData;

	//
	// Compute the vertices stating at the top pole and moving down the stacks.
	//

	// Poles: note that there will be texture coordinate distortion as there is
	// not a unique point on the texture map to assign to the pole when mapping
	// a rectangular texture onto a sphere.
	FullVertex topFullVertex{ { 0.0f, +radius, 0.0f }, { 1.f, 0.f, 0.f }, { 0.f, 0.f, -1.f }, { 0.f, +1.f, 0.f }, { 0.f, 0.f } };
	FullVertex bottomFullVertex{ { 0.0f, -radius, 0.0f }, {1.f, 0.f, 0.f}, {0.f, 0.f, -1.f}, {0.f, -1.f, 0.f }, { 0.f, 1.f } };

	meshData.Vertices.push_back(topFullVertex);

	float phiStep = M_PIF / stackCount;
	float thetaStep = 2.0f * M_PIF / sliceCount;

	// Compute vertices for each stack ring (do not count the poles as rings).
	for (UINT i = 1; i <= stackCount - 1; ++i)
	{
		float phi = i * phiStep;

		// Vertices of ring.
		for (UINT j = 0; j <= sliceCount; ++j)
		{
			float theta = j * thetaStep;

			FullVertex v;

			// spherical to cartesian
			v.PosL.x = radius * sinf(phi) * cosf(theta);
			v.PosL.y = radius * cosf(phi);
			v.PosL.z = radius * sinf(phi) * sinf(theta);

			// Partial derivative of P with respect to theta
			v.TangentL.x = -radius * sinf(phi) * sinf(theta);
			v.TangentL.y = 0.0f;
			v.TangentL.z = +radius * sinf(phi) * cosf(theta);

			v.TangentL.safenorm();

			v.NormalL = floaty3::SafelyNormalized(v.PosL);
			v.BinormalL = v.NormalL.cross(v.TangentL);

			v.Tex.x = theta / (M_PIF * 2.f);
			v.Tex.y = phi / M_PIF;

			meshData.Vertices.push_back(v);
		}
	}

	meshData.Vertices.push_back(bottomFullVertex);

	//
	// Compute indices for top stack.  The top stack was written first to the FullVertex buffer
	// and connects the top pole to the first ring.
	//

	for (UINT i = 1; i <= sliceCount; ++i)
	{
		meshData.Indices.push_back(0);
		meshData.Indices.push_back(i + 1);
		meshData.Indices.push_back(i);
	}

	//
	// Compute indices for inner stacks (not connected to poles).
	//

	// Offset the indices to the index of the first FullVertex in the first ring.
	// This is just skipping the top pole FullVertex.
	UINT baseIndex = 1;
	UINT ringFullVerTexount = sliceCount + 1;
	for (UINT i = 0; i < stackCount - 2; ++i)
	{
		for (UINT j = 0; j < sliceCount; ++j)
		{
			meshData.Indices.push_back(baseIndex + i * ringFullVerTexount + j);
			meshData.Indices.push_back(baseIndex + i * ringFullVerTexount + j + 1);
			meshData.Indices.push_back(baseIndex + (i + 1) * ringFullVerTexount + j);

			meshData.Indices.push_back(baseIndex + (i + 1) * ringFullVerTexount + j);
			meshData.Indices.push_back(baseIndex + i * ringFullVerTexount + j + 1);
			meshData.Indices.push_back(baseIndex + (i + 1) * ringFullVerTexount + j + 1);
		}
	}

	//
	// Compute indices for bottom stack.  The bottom stack was written last to the FullVertex buffer
	// and connects the bottom pole to the bottom ring.
	//

	// South pole FullVertex was added last.
	UINT southPoleIndex = (UINT)meshData.Vertices.size() - 1;

	// Offset the indices to the index of the first FullVertex in the last ring.
	baseIndex = southPoleIndex - ringFullVerTexount;

	for (UINT i = 0; i < sliceCount; ++i)
	{
		meshData.Indices.push_back(southPoleIndex);
		meshData.Indices.push_back(baseIndex + i);
		meshData.Indices.push_back(baseIndex + i + 1);
	}
	return meshData;
}

void GeoGen::GeometryGenerator::Subdivide(GeoGen::MeshData &meshData)
{
	// Save a copy of the input geometry.
	MeshData inputCopy = meshData;


	meshData.Vertices.resize(0);
	meshData.Indices.resize(0);

	//       v1
	//       *
	//      / \
			//     /   \
	//  m0*-----*m1
//   / \   / \
	//  /   \ /   \
	// *-----*-----*
// v0    m2     v2

	auto numTris = inputCopy.Indices.size() / 3;
	for (UINT i = 0ul; i < numTris; ++i)
	{
		FullVertex v0 = inputCopy.Vertices[inputCopy.Indices[i * 3 + 0]];
		FullVertex v1 = inputCopy.Vertices[inputCopy.Indices[i * 3 + 1]];
		FullVertex v2 = inputCopy.Vertices[inputCopy.Indices[i * 3 + 2]];

		//
		// Generate the midpoints.
		//

		FullVertex m0, m1, m2;

		// For subdivision, we just care about the.PosL component.  We derive the other
		// FullVertex components in CreateGeosphere.

		m0.PosL = floaty3(
			0.5f * (v0.PosL.x + v1.PosL.x),
			0.5f * (v0.PosL.y + v1.PosL.y),
			0.5f * (v0.PosL.z + v1.PosL.z));

		m1.PosL = floaty3(
			0.5f * (v1.PosL.x + v2.PosL.x),
			0.5f * (v1.PosL.y + v2.PosL.y),
			0.5f * (v1.PosL.z + v2.PosL.z));

		m2.PosL = floaty3(
			0.5f * (v0.PosL.x + v2.PosL.x),
			0.5f * (v0.PosL.y + v2.PosL.y),
			0.5f * (v0.PosL.z + v2.PosL.z));

		//
		// Add new geometry.
		//

		meshData.Vertices.push_back(v0); // 0
		meshData.Vertices.push_back(v1); // 1
		meshData.Vertices.push_back(v2); // 2
		meshData.Vertices.push_back(m0); // 3
		meshData.Vertices.push_back(m1); // 4
		meshData.Vertices.push_back(m2); // 5

		meshData.Indices.push_back(i * 6 + 0);
		meshData.Indices.push_back(i * 6 + 3);
		meshData.Indices.push_back(i * 6 + 5);

		meshData.Indices.push_back(i * 6 + 3);
		meshData.Indices.push_back(i * 6 + 4);
		meshData.Indices.push_back(i * 6 + 5);

		meshData.Indices.push_back(i * 6 + 5);
		meshData.Indices.push_back(i * 6 + 4);
		meshData.Indices.push_back(i * 6 + 2);

		meshData.Indices.push_back(i * 6 + 3);
		meshData.Indices.push_back(i * 6 + 1);
		meshData.Indices.push_back(i * 6 + 4);
	}
}

GeoGen::MeshData GeoGen::GeometryGenerator::CreateGeosphere(float radius, UINT numSubdivisions)
{
	MeshData meshData;
	// Put a cap on the number of subdivisions.
	numSubdivisions = std::min(numSubdivisions, 5u);

	// Approximate a sphere by tessellating an icosahedron.

	const float X = 0.525731f;
	const float Z = 0.850651f;

	floaty3 pos[12] =
	{
		floaty3(-X, 0.0f, Z),  floaty3(X, 0.0f, Z),
		floaty3(-X, 0.0f, -Z), floaty3(X, 0.0f, -Z),
		floaty3(0.0f, Z, X),   floaty3(0.0f, Z, -X),
		floaty3(0.0f, -Z, X),  floaty3(0.0f, -Z, -X),
		floaty3(Z, X, 0.0f),   floaty3(-Z, X, 0.0f),
		floaty3(Z, -X, 0.0f),  floaty3(-Z, -X, 0.0f)
	};

	uint32_t k[60] =
	{
		1,4,0,  4,9,0,  4,5,9,  8,5,4,  1,8,4,
		1,10,8, 10,3,8, 8,3,5,  3,2,5,  3,7,2,
		3,10,7, 10,6,7, 6,11,7, 6,0,11, 6,1,0,
		10,1,6, 11,0,9, 2,11,9, 5,2,9,  11,2,7
	};

	meshData.Vertices.resize(12);
	meshData.Indices.resize(60);

	for (UINT i = 0; i < 12; ++i)
		meshData.Vertices[i].PosL = pos[i];

	for (UINT i = 0; i < 60; ++i)
		meshData.Indices[i] = k[i];

	for (UINT i = 0; i < numSubdivisions; ++i)
		Subdivide(meshData);

	// Project vertices onto sphere and scale.
	for (UINT i = 0; i < meshData.Vertices.size(); ++i)
	{
		// Chuck onto unit circle, ready for next step (multiply)
		floaty3 n = floaty3::SafelyNormalized(meshData.Vertices[i].PosL);

		// Chuck onto sphere
		meshData.Vertices[i].PosL = radius * n;
		meshData.Vertices[i].NormalL = n;
		
		// Derive texture coordinates from spherical coordinates.
		float theta = AngleFromXY(
			meshData.Vertices[i].PosL.x,
			meshData.Vertices[i].PosL.z);

		float phi = acosf(meshData.Vertices[i].PosL.y / radius);

		meshData.Vertices[i].Tex.x = theta / (M_PIF * 2.f);
		meshData.Vertices[i].Tex.y = phi / M_PIF;

		// Partial derivative of P with respect to theta
		meshData.Vertices[i].TangentL.x = -radius * sinf(phi) * sinf(theta);
		meshData.Vertices[i].TangentL.y = 0.0f;
		meshData.Vertices[i].TangentL.z = +radius * sinf(phi) * cosf(theta);

		meshData.Vertices[i].TangentL.safenorm();
		meshData.Vertices[i].BinormalL = meshData.Vertices[i].NormalL.cross(meshData.Vertices[i].TangentL);
	}
	return meshData;
}

GeoGen::MeshData GeoGen::GeometryGenerator::CreateCylinder(float bottomRadius, float topRadius, float height, UINT sliceCount, UINT stackCount)
{
	MeshData meshData;
	meshData.Vertices.clear();
	meshData.Indices.clear();

	float stackHeight = height / stackCount;

	// Amount to increment radii as we move up each stack level from bottom to top.
	float radiusStep = (topRadius - bottomRadius) / stackCount;

	UINT ringCount = stackCount + 1;

	// Compute vertices for each stack ring starting at the bottom and moving up.
	for (UINT i = 0; i < ringCount; ++i)
	{
		float y = -0.5f * height + i * stackHeight;
		float r = bottomRadius + i * radiusStep;

		// vertices of ring
		float dTheta = 2.0f * M_PIF / sliceCount;
		for (UINT j = 0; j <= sliceCount; ++j)
		{
			FullVertex vertex;

			float c = cosf(j * dTheta);
			float s = sinf(j * dTheta);

			vertex.PosL = floaty3(r * c, y, r * s);

			vertex.Tex.x = (float)j / sliceCount;
			vertex.Tex.y = 1.0f - (float)i / stackCount;

			// Cylinder can be parameterized as follows, where we introduce v
			// parameter that goes in the same direction as the v tex-coord
			// so that the bitangent goes in the same direction as the v tex-coord.
			//   Let r0 be the bottom radii and let r1 be the top radii.
			//   y(v) = h - hv for v in [0,1].
			//   r(v) = r1 + (r0-r1)v
			//
			//   x(t, v) = r(v)*cos(t)
			//   y(t, v) = h - hv
			//   z(t, v) = r(v)*sin(t)
			// 
			//  dx/dt = -r(v)*sin(t)
			//  dy/dt = 0
			//  dz/dt = +r(v)*cos(t)
			//
			//  dx/dv = (r0-r1)*cos(t)
			//  dy/dv = -h
			//  dz/dv = (r0-r1)*sin(t)

			// This is unit length.
			vertex.TangentL = floaty3(-s, 0.0f, c);

			float dr = bottomRadius - topRadius;
			vertex.BinormalL = { dr * c, -height, dr * s };
			
			vertex.NormalL = floaty3::SafelyNormalized(vertex.TangentL.cross(vertex.BinormalL));

			meshData.Vertices.push_back(vertex);
		}
	}

	// Add one because we duplicate the first and last FullVertex per ring
	// since the texture coordinates are different.
	UINT ringFullVerTexount = sliceCount + 1;

	// Compute indices for each stack.
	for (UINT i = 0; i < stackCount; ++i)
	{
		for (UINT j = 0; j < sliceCount; ++j)
		{
			meshData.Indices.push_back(i * ringFullVerTexount + j);
			meshData.Indices.push_back((i + 1) * ringFullVerTexount + j);
			meshData.Indices.push_back((i + 1) * ringFullVerTexount + j + 1);

			meshData.Indices.push_back(i * ringFullVerTexount + j);
			meshData.Indices.push_back((i + 1) * ringFullVerTexount + j + 1);
			meshData.Indices.push_back(i * ringFullVerTexount + j + 1);
		}
	}

	BuildCylinderTopCap(bottomRadius, topRadius, height, sliceCount, stackCount, meshData);
	BuildCylinderBottomCap(bottomRadius, topRadius, height, sliceCount, stackCount, meshData);
	return meshData;
}

#pragma warning(suppress:4100)
void GeoGen::GeometryGenerator::BuildCylinderTopCap(float bottomRadius, float topRadius, float height, UINT sliceCount, UINT stackCount, GeoGen::MeshData &meshData)
{
	UINT baseIndex = (UINT)meshData.Vertices.size();

	float y = 0.5f * height;
	float dTheta = 2.0f * M_PIF / sliceCount;

	// Duplicate cap ring vertices because the texture coordinates and normals differ.
	for (UINT i = 0; i <= sliceCount; ++i)
	{
		float x = topRadius * cosf(i * dTheta);
		float z = topRadius * sinf(i * dTheta);

		// Scale down by the height to try and make top cap texture coord area
		// proportional to base.
		float u = x / height + 0.5f;
		float v = z / height + 0.5f;

		meshData.Vertices.push_back(FullVertex{{ x, y, z }, { 1.f, 0.f, 0.f }, { 0.f, 0.f, -1.f }, { 0.0f, 1.0f, 0.0f }, { u, v }});
	}

	// Cap center FullVertex.
	meshData.Vertices.push_back(FullVertex{ {0.0f, y, 0.0f}, {1.f, 0.f, 0.f}, {0.f, 0.f, -1.f}, {0.f, 1.f, 0.f}, {0.5f, 0.5f} });

	// Index of center FullVertex.
	UINT centerIndex = (UINT)meshData.Vertices.size() - 1;

	for (UINT i = 0; i < sliceCount; ++i)
	{
		meshData.Indices.push_back(centerIndex);
		meshData.Indices.push_back(baseIndex + i + 1);
		meshData.Indices.push_back(baseIndex + i);
	}
}

#pragma warning(suppress:4100)
void GeoGen::GeometryGenerator::BuildCylinderBottomCap(float bottomRadius, float topRadius, float height, UINT sliceCount, UINT stackCount, GeoGen::MeshData &meshData)
{
	// 
	// Build bottom cap.
	//

	UINT baseIndex = (UINT)meshData.Vertices.size();
	float y = -0.5f * height;

	// vertices of ring
	float dTheta = 2.0f * M_PIF / sliceCount;
	for (UINT i = 0; i <= sliceCount; ++i)
	{
		float x = bottomRadius * cosf(i * dTheta);
		float z = bottomRadius * sinf(i * dTheta);

		// Scale down by the height to try and make top cap texture coord area
		// proportional to base.
		float u = x / height + 0.5f;
		float v = z / height + 0.5f;

		meshData.Vertices.push_back(FullVertex{ {x, y, z}, {1.f, 0.f, 0.f}, {0.f, 0.f, 1.f}, {0.f, -1.0f, 0.0f}, {u, v} });
	}

	// Cap center FullVertex.
	meshData.Vertices.push_back(FullVertex{ {0.0f, y, 0.0f}, {1.f, 0.f, 0.f}, { 0.f, 0.f, 1.f}, {0.0f, -1.0f, 0.0f}, {0.5f, 0.5f } });

	// Cache the index of center FullVertex.
	UINT centerIndex = (UINT)meshData.Vertices.size() - 1;

	for (UINT i = 0; i < sliceCount; ++i)
	{
		meshData.Indices.push_back(centerIndex);
		meshData.Indices.push_back(baseIndex + i);
		meshData.Indices.push_back(baseIndex + i + 1);
	}
}

GeoGen::MeshData GeoGen::GeometryGenerator::CreateGrid(float width, float depth, UINT m, UINT n)
{
	GeoGen::MeshData meshData;

	UINT FullVerTexount = m * n;
	UINT faceCount = (m - 1) * (n - 1) * 2;

	//
	// Create the vertices.
	//

	float halfWidth = 0.5f * width;
	float halfDepth = 0.5f * depth;

	float dx = width / (n - 1);
	float dz = depth / (m - 1);

	float du = 1.0f / (n - 1);
	float dv = 1.0f / (m - 1);

	meshData.Vertices.resize(FullVerTexount);
	for (UINT i = 0; i < m; ++i)
	{
		float z = halfDepth - i * dz;
		for (UINT j = 0; j < n; ++j)
		{
			float x = -halfWidth + j * dx;

			meshData.Vertices[i * n + j].PosL = floaty3(x, 0.0f, z);
			meshData.Vertices[i * n + j].NormalL = floaty3(0.0f, 1.0f, 0.0f);
			meshData.Vertices[i * n + j].TangentL = floaty3(1.0f, 0.0f, 0.0f);

			// Stretch texture over grid.
			meshData.Vertices[i * n + j].Tex.x = j * du;
			meshData.Vertices[i * n + j].Tex.y = i * dv;
		}
	}

	//
	// Create the indices.
	//

	meshData.Indices.resize(faceCount * 3); // 3 indices per face

											// Iterate over each quad and compute indices.
	UINT k = 0;
	for (UINT i = 0; i < m - 1; ++i)
	{
		for (UINT j = 0; j < n - 1; ++j)
		{
			meshData.Indices[k] = i * n + j;
			meshData.Indices[k + 1] = i * n + j + 1;
			meshData.Indices[k + 2] = (i + 1) * n + j;

			meshData.Indices[k + 3] = (i + 1) * n + j;
			meshData.Indices[k + 4] = i * n + j + 1;
			meshData.Indices[k + 5] = (i + 1) * n + j + 1;

			k += 6; // next quad
		}
	}
	return meshData;
}

/*
Geometry::Shape GeoGen::GeometryGenerator::CreateBoxS(float width, float height, float depth, Geometry::Material mat)
{
	MeshData meshversion = CreateBox(width, height, depth);

	Geometry::Shape shapeversion = Geometry::Shape();
	shapeversion.Material = mat;
	shapeversion.Indices.resize(meshversion.Indices.size());
	shapeversion.Vertices.resize(meshversion.Vertices.size());

	for (auto &ass : meshversion.Indices)
	{
		shapeversion.Indices.push_back(ass);
	}

	for (auto &ass : meshversion.Vertices)
	{
		XMVECTOR binormalxm = XMVector3Cross(XMLoadFloat3(&ass.TangentL), XMLoadFloat3(&ass.NormalL));
		floaty3 binormal;
		XMStoreFloat3(&binormal, binormalxm);
		shapeversion.Vertices.push_back({ ass.PosL, ass.TangentL, binormal, ass.NormalL, ass.Tex });
	}
	return shapeversion;
}

Geometry::Shape GeoGen::GeometryGenerator::CreateSphereS(float radius, UINT sliceCount, UINT stackCount, Geometry::Material mat)
{
	MeshData meshversion = CreateSphere(radius, sliceCount, stackCount);

	Geometry::Shape shapeversion = Geometry::Shape();
	shapeversion.Material = mat;
	shapeversion.Indices.resize(meshversion.Indices.size());
	shapeversion.Vertices.resize(meshversion.Vertices.size());

	for (auto &ass : meshversion.Indices)
	{
		shapeversion.Indices.push_back(ass);
	}

	for (auto &ass : meshversion.Vertices)
	{
		XMVECTOR binormalxm = XMVector3Cross(XMLoadFloat3(&ass.TangentL), XMLoadFloat3(&ass.NormalL));
		floaty3 binormal;
		XMStoreFloat3(&binormal, binormalxm);
		shapeversion.Vertices.push_back({ ass.PosL, ass.TangentL, binormal, ass.NormalL, ass.Tex });
	}
	return shapeversion;
}

Geometry::Shape GeoGen::GeometryGenerator::CreateGeosphereS(float radius, UINT numSubdivisions, Geometry::Material mat)
{
	MeshData meshversion = CreateGeosphere(radius, numSubdivisions);

	Geometry::Shape shapeversion = Geometry::Shape();
	shapeversion.Material = mat;
	shapeversion.Indices.resize(meshversion.Indices.size());
	shapeversion.Vertices.resize(meshversion.Vertices.size());

	for (UINT i = 0; i < (UINT)meshversion.Indices.size(); ++i)
	{
		shapeversion.Indices[i] = meshversion.Indices[i];
	}

	for (UINT i = 0; i < (UINT)meshversion.Vertices.size(); ++i)
	{
		XMVECTOR binormalxm = XMVector3Cross(XMLoadFloat3(&meshversion.Vertices[i].TangentL), XMLoadFloat3(&meshversion.Vertices[i].NormalL));
		floaty3 binormal;
		XMStoreFloat3(&binormal, binormalxm);
		shapeversion.Vertices[i] = { meshversion.Vertices[i].PosL, meshversion.Vertices[i].TangentL, binormal, meshversion.Vertices[i].NormalL, meshversion.Vertices[i].Tex };
	}
	return shapeversion;
}

Geometry::Shape GeoGen::GeometryGenerator::CreateCylinderS(float bottomRadius, float topRadius, float height, UINT sliceCount, UINT stackCount, Geometry::Material mat)
{
	GeoGen::MeshData meshversion = CreateCylinder(bottomRadius, topRadius, height, sliceCount, stackCount);

	Geometry::Shape shapeversion = Geometry::Shape();
	shapeversion.Material = mat;
	shapeversion.Indices.resize(meshversion.Indices.size());
	shapeversion.Vertices.resize(meshversion.Vertices.size());

	for (auto &ass : meshversion.Indices)
	{
		shapeversion.Indices.push_back(ass);
	}

	for (auto &ass : meshversion.Vertices)
	{
		XMVECTOR binormalxm = XMVector3Cross(XMLoadFloat3(&ass.TangentL), XMLoadFloat3(&ass.NormalL));
		floaty3 binormal;
		XMStoreFloat3(&binormal, binormalxm);
		shapeversion.Vertices.push_back({ ass.PosL, ass.TangentL, binormal, ass.NormalL, ass.Tex });
	}
	return shapeversion;
}

Geometry::Shape GeoGen::GeometryGenerator::CreateGridS(float width, float depth, UINT m, UINT n, Geometry::Material mat)
{
	GeoGen::MeshData meshversion = CreateGrid(width, depth, m, n);

	Geometry::Shape shapeversion = Geometry::Shape();
	shapeversion.Material = mat;
	shapeversion.Indices.resize(meshversion.Indices.size());
	shapeversion.Vertices.resize(meshversion.Vertices.size());

	for (auto &ass : meshversion.Indices)
	{
		shapeversion.Indices.push_back(ass);
	}

	for (auto &ass : meshversion.Vertices)
	{
		XMVECTOR binormalxm = XMVector3Cross(XMLoadFloat3(&ass.TangentL), XMLoadFloat3(&ass.NormalL));
		floaty3 binormal;
		XMStoreFloat3(&binormal, binormalxm);
		shapeversion.Vertices.push_back({ ass.PosL, ass.TangentL, binormal, ass.NormalL, ass.Tex });
	}
	return shapeversion;
}

void GeometryGenerator::CreateFullscreenQuad(MeshData &meshData)
{
	meshData.Vertices.resize(4);
	meshData.Indices.resize(6);

	//.PosL coordinates specified in NDC space.
	meshData.Vertices[0] = FullVertex(
		-1.0f, -1.0f, 0.0f,
		0.0f, 0.0f, -1.0f,
		1.0f, 0.0f, 0.0f,
		0.0f, 1.0f);

	meshData.Vertices[1] = FullVertex(
		-1.0f, +1.0f, 0.0f,
		0.0f, 0.0f, -1.0f,
		1.0f, 0.0f, 0.0f,
		0.0f, 0.0f);

	meshData.Vertices[2] = FullVertex(
		+1.0f, +1.0f, 0.0f,
		0.0f, 0.0f, -1.0f,
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f);

	meshData.Vertices[3] = FullVertex(
		+1.0f, -1.0f, 0.0f,
		0.0f, 0.0f, -1.0f,
		1.0f, 0.0f, 0.0f,
		1.0f, 1.0f);

	meshData.Indices[0] = 0;
	meshData.Indices[1] = 1;
	meshData.Indices[2] = 2;

	meshData.Indices[3] = 0;
	meshData.Indices[4] = 2;
	meshData.Indices[5] = 3;
}*/


float GeoGen::AngleFromXY(float x, float y)
{
	float angle = 0.0f;

	if (x >= 0.0f)
	{
		// Its in Quadrant 1 or Quadrant 2, basically positive x
		angle = atanf(y / x);

		if (angle < 0.0f)
			angle += 6.28318531f; // 2 pi

	}
	else // Its in Quadrant 3 or Quadrant 4, negative x
		angle = atanf(y / x) + M_PIF;

	return angle;
}