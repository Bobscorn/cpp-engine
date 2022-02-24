#include "VoxelCube.h"

#include "Drawing/IRen3D.h"

#include "Structure/BulletBitmasks.h"

std::weak_ptr<Material> Voxel::VoxelCube::s_Mats[5];
std::weak_ptr<GeoThing> Voxel::VoxelCube::s_Geo;
std::weak_ptr<btBoxShape> Voxel::VoxelCube::s_Shape;

constexpr std::array<FullVertex, 24> CubeVertices = {
			FullVertex{ floaty3(+0.500000f, +0.500000f, +0.500000f), floaty3(-0.000000f, -1.000000f, +0.000000f), floaty3(-1.000000f, +0.000000f, +0.000000f), floaty3(+0.000000f, +0.000000f, +1.000000f), floaty2(+0.000000f, +1.000000f) },
			FullVertex{ floaty3(-0.500000f, -0.500000f, +0.500000f), floaty3(-0.000000f, -1.000000f, +0.000000f), floaty3(-1.000000f, +0.000000f, +0.000000f), floaty3(+0.000000f, +0.000000f, +1.000000f), floaty2(+1.000000f, +0.000000f) },
			FullVertex{ floaty3(-0.500000f, +0.500000f, +0.500000f), floaty3(-0.000000f, -1.000000f, +0.000000f), floaty3(-1.000000f, +0.000000f, +0.000000f), floaty3(+0.000000f, +0.000000f, +1.000000f), floaty2(+0.000000f, +0.000000f) },
			FullVertex{ floaty3(-0.500000f, +0.500000f, -0.500000f), floaty3(-1.000000f, +0.000000f, +0.000000f), floaty3(-0.000000f, -1.000000f, +0.000000f), floaty3(+0.000000f, +0.000000f, -1.000000f), floaty2(+1.000000f, +1.000000f) },
			FullVertex{ floaty3(+0.500000f, -0.500001f, -0.500000f), floaty3(-1.000000f, +0.000000f, +0.000000f), floaty3(-0.000000f, -1.000000f, +0.000000f), floaty3(+0.000000f, +0.000000f, -1.000000f), floaty2(+0.000000f, +0.000000f) },
			FullVertex{ floaty3(+0.500000f, +0.500000f, -0.500000f), floaty3(-1.000000f, +0.000000f, +0.000000f), floaty3(-0.000000f, -1.000000f, +0.000000f), floaty3(+0.000000f, +0.000000f, -1.000000f), floaty2(+0.000000f, +1.000000f) },
			FullVertex{ floaty3(+0.500000f, +0.500000f, -0.500000f), floaty3(+0.000000f, -0.000000f, -1.000000f), floaty3(-0.000000f, -1.000000f, +0.000000f), floaty3(+1.000000f, +0.000000f, +0.000000f), floaty2(+1.000000f, +1.000000f) },
			FullVertex{ floaty3(+0.500000f, -0.500000f, +0.500000f), floaty3(+0.000000f, -0.000000f, -1.000000f), floaty3(-0.000000f, -1.000000f, +0.000000f), floaty3(+1.000000f, +0.000000f, +0.000000f), floaty2(+0.000000f, +0.000000f) },
			FullVertex{ floaty3(+0.500000f, +0.500000f, +0.500000f), floaty3(+0.000000f, -0.000000f, -1.000000f), floaty3(+0.000000f, -1.000000f, +0.000000f), floaty3(+1.000000f, +0.000000f, +0.000000f), floaty2(+0.000000f, +1.000000f) },
			FullVertex{ floaty3(+0.500000f, -0.500001f, -0.500000f), floaty3(-0.000000f, -0.000000f, -1.000000f), floaty3(-1.000000f, +0.000000f, +0.000000f), floaty3(+0.000000f, -1.000000f, +0.000000f), floaty2(+1.000000f, +1.000000f) },
			FullVertex{ floaty3(-0.500000f, -0.500000f, +0.500000f), floaty3(-0.000000f, -0.000000f, -1.000000f), floaty3(-1.000000f, +0.000000f, +0.000000f), floaty3(+0.000000f, -1.000000f, +0.000000f), floaty2(+0.000000f, +0.000000f) },
			FullVertex{ floaty3(+0.500000f, -0.500000f, +0.500000f), floaty3(-0.000000f, -0.000000f, -1.000000f), floaty3(-1.000000f, +0.000000f, +0.000000f), floaty3(+0.000000f, -1.000000f, +0.000000f), floaty2(+0.000000f, +1.000000f) },
			FullVertex{ floaty3(-0.500000f, -0.500000f, +0.500000f), floaty3(-0.000000f, +0.000000f, -1.000000f), floaty3(+0.000000f, +1.000000f, -0.000000f), floaty3(-1.000000f, +0.000000f, +0.000000f), floaty2(+0.000000f, +1.000000f) },
			FullVertex{ floaty3(-0.500000f, +0.500000f, -0.500000f), floaty3(-0.000000f, +0.000000f, -1.000000f), floaty3(+0.000000f, +1.000000f, -0.000000f), floaty3(-1.000000f, +0.000000f, +0.000000f), floaty2(+1.000000f, +0.000000f) },
			FullVertex{ floaty3(-0.500000f, +0.500000f, +0.500000f), floaty3(-0.000000f, +0.000000f, -1.000000f), floaty3(+0.000000f, +1.000000f, -0.000000f), floaty3(-1.000000f, +0.000000f, +0.000000f), floaty2(+0.000000f, +0.000000f) },
			FullVertex{ floaty3(+0.500000f, +0.500000f, +0.500000f), floaty3(+0.000000f, +0.000000f, +1.000000f), floaty3(-1.000000f, +0.000000f, -0.000000f), floaty3(+0.000000f, +1.000000f, -0.000000f), floaty2(+1.000000f, +1.000000f) },
			FullVertex{ floaty3(-0.500000f, +0.500000f, -0.500000f), floaty3(+0.000000f, +0.000000f, +1.000000f), floaty3(-1.000000f, +0.000000f, -0.000000f), floaty3(+0.000000f, +1.000000f, -0.000000f), floaty2(+0.000000f, +0.000000f) },
			FullVertex{ floaty3(+0.500000f, +0.500000f, -0.500000f), floaty3(+0.000000f, +0.000000f, +1.000000f), floaty3(-1.000000f, +0.000000f, -0.000000f), floaty3(+0.000000f, +1.000000f, -0.000000f), floaty2(+0.000000f, +1.000000f) },
			FullVertex{ floaty3(+0.500000f, -0.500000f, +0.500000f), floaty3(+0.000000f, -1.000000f, +0.000000f), floaty3(-1.000000f, +0.000000f, +0.000000f), floaty3(+0.000000f, +0.000000f, +1.000000f), floaty2(+1.000000f, +1.000000f) },
			FullVertex{ floaty3(-0.500000f, -0.500000f, -0.500000f), floaty3(-1.000000f, +0.000001f, +0.000000f), floaty3(+0.000000f, -1.000000f, +0.000000f), floaty3(+0.000000f, +0.000000f, -1.000000f), floaty2(+1.000000f, +0.000000f) },
			FullVertex{ floaty3(+0.500000f, -0.500001f, -0.500000f), floaty3(-0.000000f, -0.000000f, -1.000000f), floaty3(-0.000001f, -1.000000f, +0.000000f), floaty3(+1.000000f, -0.000001f, -0.000000f), floaty2(+1.000000f, +0.000000f) },
			FullVertex{ floaty3(-0.500000f, -0.500000f, -0.500000f), floaty3(-0.000000f, +0.000000f, -1.000000f), floaty3(-1.000000f, +0.000000f, +0.000000f), floaty3(-0.000000f, -1.000000f, -0.000000f), floaty2(+1.000000f, +0.000000f) },
			FullVertex{ floaty3(-0.500000f, -0.500000f, -0.500000f), floaty3(-0.000000f, +0.000000f, -1.000000f), floaty3(+0.000000f, +1.000000f, -0.000000f), floaty3(-1.000000f, +0.000000f, +0.000000f), floaty2(+1.000000f, +1.000000f) },
			FullVertex{ floaty3(-0.500000f, +0.500000f, +0.500000f), floaty3(+0.000000f, -0.000000f, +1.000000f), floaty3(-1.000000f, +0.000000f, +0.000000f), floaty3(+0.000000f, +1.000000f, -0.000000f), floaty2(+1.000000f, +0.000000f) }
};
constexpr std::array<unsigned int, 36> CubeIndices = { 2, 1, 0, 5, 4, 3, 8, 7, 6, 11, 10, 9, 14, 13, 12, 17, 16, 15, 1, 18, 0, 4, 19, 3, 7, 20, 6, 10, 21, 9, 13, 22, 12, 16, 23, 15, };

Voxel::VoxelCube::VoxelCube(G1::IGSpace *container, CommonResources *resources, VoxelWorld *world, floaty3 position, size_t x, size_t y, size_t z, Voxel::ChunkyFrustumCuller *chunkyboi) : IShape(container, "Cube"), ICube(world), FullResourceHolder(resources), m_Geo(GetGeo()), m_Mat(GetMat()), m_Shape(GetShape()), m_Body(GetBody(m_Shape.get())), m_Trans(Matrixy4x4::Translate(position)), m_X(x), m_Y(y), m_Z(z), m_Culler((chunkyboi ? std::make_unique<BlockCuller>(m_X, m_Y, m_Z, chunkyboi) : nullptr))
{
	btTransform tmp;
	tmp.setFromOpenGLMatrix(m_Trans.ma);
	m_Body->setWorldTransform(tmp);

	Drawing::DrawCall call;
	call.DebugString = "Voxel Cube";
	call.Enabled = true;
	call.Geometry = m_Geo;
	call.Material = m_Mat;
	call.Matrix = &m_Trans;

	if (m_Culler)
		call.FrustumCuller = m_Culler.get();

	m_DrawKey = mResources->Ren3->AddDrawCall(call);

	Container->RequestPhysicsCall(m_Body, ENVIRONMENT, PLAYER | ENTITY_GENERAL);
}

Voxel::VoxelCube::~VoxelCube()
{
	mResources->Ren3->RemoveDrawCall(m_DrawKey);
}

void Voxel::VoxelCube::BeforeDraw()
{
}

void Voxel::VoxelCube::AfterDraw()
{
}

std::shared_ptr<GeoThing> Voxel::VoxelCube::GetGeo()
{
	auto geo = s_Geo.lock();
	if (geo)
	{
		return geo;
	}
	
	if constexpr (Voxel::BlockSize == 1.f)
	{
		geo = mResources->Ren3->AddGeometry({ CubeVertices.cbegin(), CubeVertices.cend() }, { CubeIndices.cbegin(), CubeIndices.cend() });
		s_Geo = geo;
	}
	else
	{
		std::vector<FullVertex> vertices;
		vertices.reserve(CubeVertices.size());
		
		for (size_t i = 0; i < CubeVertices.size(); ++i)
		{
			vertices.push_back(CubeVertices[i]);
			vertices[i].PosL *= Voxel::BlockSize;
		}

		geo = mResources->Ren3->AddGeometry(vertices, { CubeIndices.cbegin(), CubeIndices.cend() });
		s_Geo = geo;
	}

	return geo;
}

floaty3 Voxel::VoxelCube::GetPosition() const
{
	return { m_Trans.dx, m_Trans.dy, m_Trans.dz };
}

void Voxel::VoxelCube::SetCull(ChunkyFrustumCuller *chunkyboi)
{
	if (chunkyboi)
	{
		m_Culler = std::make_unique<BlockCuller>(m_X, m_Y, m_Z, chunkyboi);

		mResources->Ren3->GetDrawCall(m_DrawKey)->FrustumCuller = m_Culler.get();
	}
	else
		ResetCuller();
}

void Voxel::VoxelCube::SetChunkPosition(size_t x, size_t y, size_t z)
{
	m_X = x; m_Y = y; m_Z = z;
	if (m_Culler)
	{
		ResetCuller();
	}
}

void Voxel::VoxelCube::UpdatePosition(floaty3 new_position, size_t new_x, size_t new_y, size_t new_z, ChunkyFrustumCuller *new_culler)
{
	m_Trans = Matrixy4x4::Translate(new_position);
	btTransform tmp;
	tmp.setFromOpenGLMatrix(m_Trans.ma);
	m_Body->setWorldTransform(tmp);
	SetChunkPosition(new_x, new_y, new_z);
	SetCull(new_culler);
}

void Voxel::VoxelCube::ResetCuller()
{
	m_Culler.reset();

	mResources->Ren3->GetDrawCall(m_DrawKey)->FrustumCuller = nullptr;
}

std::shared_ptr<Material> Voxel::VoxelCube::GetMat()
{
	size_t index = (m_X + m_Y + m_Z) % 5;
	auto mat = s_Mats[index].lock();
	if (mat)
	{
		return mat;
	}

	mat = std::make_shared<Material>(s_MaterialDatas[index]);
	s_Mats[index] = mat;
	return mat;
}

std::shared_ptr<btBoxShape> Voxel::VoxelCube::GetShape()
{
	auto shape = s_Shape.lock();
	if (shape)
		return shape;

	shape = std::make_shared<btBoxShape>(btVector3{ 0.5f, 0.5f, 0.5f });
	s_Shape = shape;
	return shape;
}

std::shared_ptr<btCollisionObject> Voxel::VoxelCube::GetBody(btCollisionShape *shape)
{
	auto out = std::make_shared<btCollisionObject>();
	out->setUserPointer(&this->m_Holder);
	out->setCollisionShape(shape);
	return out;
}
