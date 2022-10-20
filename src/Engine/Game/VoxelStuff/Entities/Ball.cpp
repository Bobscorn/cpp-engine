#include "Ball.h"

#include "Game/VoxelStuff/VoxelWorld.h"

#include "Structure/BulletBitmasks.h"

#include "Helpers/BulletHelper.h"
#include "Helpers/DrawingHelper.h"

#include "Drawing/FreeMaterials.h"

#include "HelpfulThing.h"

Voxel::Entities::Ball::Ball(VoxelWorld *world, CommonResources *resources, floaty3 position) : Entity(world), FullResourceHolder(resources), m_Trans(Matrixy4x4::Translate(position)), m_Cull(&m_Trans, 1.f), m_DrawKey(GenerateDrawcall())
{
	GenerateBody();

	m_Body->setUserPointer(&m_Holder);

	m_World->GetContainer()->RequestPhysicsCall(m_Body, ENTITY_GENERAL, ENVIRONMENT | PLAYER);
}

Voxel::Entities::Ball::~Ball()
{
	if (m_DrawKey)
		mResources->Ren3->RemoveDrawCall(m_DrawKey);
}

floaty3 Voxel::Entities::Ball::GetCentre()
{
	return { m_Trans.dx, m_Trans.dy, m_Trans.dz };
}

void Voxel::Entities::Ball::BeforeDraw()
{
}

void Voxel::Entities::Ball::AfterDraw()
{
}

bool Voxel::Entities::Ball::HitByProjectile(Projectile *proj, floaty3 hit_point)
{
	(void)proj;
	(void)hit_point;
	return true;
}

void Voxel::Entities::Ball::TakeKnockbackDirect(floaty3 knock_back)
{
	m_Body->applyCentralImpulse(knock_back);
}

void Voxel::Entities::Ball::getWorldTransform(btTransform &out) const
{
	out.setFromOpenGLMatrix(m_Trans.ma);
}

void Voxel::Entities::Ball::setWorldTransform(const btTransform &in)
{
	in.getOpenGLMatrix(m_Trans.ma);
}

void Voxel::Entities::Ball::GenerateBody()
{
	/*IShapeRequestData data;
	Requests::Request ass = Requests::Request("GetShapeStore", {}, &data);
	mResources->Request->Request(ass);

	if (data.gotem)
	{
		DefaultShapeCreator<btSphereShape, btScalar> asss{ m_Shape, 1.f };
		m_Shape = data.gotem->GetOrCreate("BallSphere", &asss).lock();
	}

	if (!m_Shape)
	{
		m_Shape = std::make_shared<btSphereShape>(1.f);
	}*/

	m_Shape = SphereCreator<1,1>::GetShape();
	
	btVector3 local_inertia;
	m_Shape->calculateLocalInertia(2.f, local_inertia);
	m_Body = std::make_shared<btRigidBody>(btRigidBody::btRigidBodyConstructionInfo(2.f, this, m_Shape.get(), local_inertia));
}

size_t Voxel::Entities::Ball::GenerateDrawcall()
{
	Drawing::DrawCall call;
	call.Matrix = &m_Trans;
	call.Material = MatMan<0u>::Get();
	call.Geometry = GeoMan<1, 1, 10, 10>::Get(mResources->Ren3);
	//call.FrustumCuller = &m_Cull;
	call.DebugString = "Ball Entity";

	return mResources->Ren3->AddDrawCall(call);
}

Voxel::SphereCuller::SphereCuller(Matrixy4x4 *trans, float rad) : m_Trans(trans), m_Radius(rad)
{
}

bool Voxel::SphereCuller::InsideFrustum(const CameraFrustum &f)
{
	Sphere obj = { m_Trans->Transform({0.f, 0.f, 0.f}), m_Radius };

	return Voxel::InsideFrustum<Sphere, CameraFrustum>(f, obj);
}

void Voxel::SphereCuller::UpdateRadius(float rad)
{
	m_Radius = rad;
}
