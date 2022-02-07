#include "PerviousShapes.h"

#include "Helpers/MathHelper.h"
#include "Helpers/ProfileHelper.h"

#include "Config/Config.h"

#include "Systems/Input/Input.h"

#include "Systems/Requests/Requester.h"

#include <filesystem>
#include <assert.h>

const float Perviousity::Player::DefaultOutOfBounds = -25.f;

std::unique_ptr<btCylinderShape> Perviousity::Player::PerviousPlayer::Capsule = nullptr;
std::unique_ptr<btCapsuleShape> Perviousity::Player::PerviousPlayer::SweepCapsule = nullptr;
std::shared_ptr<btCollisionShape> Perviousity::Shapes::WallShape = nullptr;
std::unique_ptr<btCollisionShape> Perviousity::Shapes::FloorShape = nullptr;
std::shared_ptr<Material> Perviousity::Shapes::g_OpaqueMaterial;
std::shared_ptr<Material> Perviousity::Shapes::g_TransparentMaterial;
std::shared_ptr<Perviousity::Shapes::PlatformMaterial> Perviousity::Shapes::DefaultMaterial;
std::weak_ptr<GeoThing> Perviousity::Shapes::PerviousWall::s_PartGeos[4];
std::weak_ptr<GeoThing> Perviousity::Shapes::PerviousWall::s_OuterGeo;
std::weak_ptr<Material> Perviousity::Shapes::PerviousWall::s_DefaultMat;
//std::weak_ptr<GeoThing> Perviousity::Shapes::PerviousWall::s_WallGeo{};
//std::shared_ptr<GeoThing> WallGeometryKey = nullptr;
//std::shared_ptr<GeoThing> FloorGeometryKey = nullptr;

std::weak_ptr<GeoThing> Perviousity::Activators::StandButton::s_CaseGeo{};
std::weak_ptr<GeoThing> Perviousity::Activators::StandButton::s_RedGeo{};
std::weak_ptr<GeoThing> Perviousity::Activators::StandButton::s_StandGeo{};
std::weak_ptr<Material> Perviousity::Activators::StandButton::s_StandMat{};
std::weak_ptr<Material> Perviousity::Activators::StandButton::s_RedMat{};
std::weak_ptr<Material> Perviousity::Activators::StandButton::s_CaseMat{};
std::weak_ptr<GeoThing> Perviousity::Activators::StandButton::s_SpecialGeo{};

std::weak_ptr<GeoThing> Perviousity::Activators::PressurePad::s_PressureGeo{};
std::weak_ptr<GeoThing> Perviousity::Activators::PressurePad::s_BaseGeo{};
std::weak_ptr<GeoThing> Perviousity::Activators::PressurePad::s_SpecialGeo{};
std::weak_ptr<Material> Perviousity::Activators::PressurePad::s_BaseMat{};
std::weak_ptr<Material> Perviousity::Activators::PressurePad::s_PressureMat{};

Matrixy4x4 Perviousity::Activators::StandButton::StandMatrix = Matrixy4x4::Identity();
Matrixy4x4 Perviousity::Activators::StandButton::CaseMatrix = Matrixy4x4::Identity();
Matrixy4x4 Perviousity::Activators::StandButton::RedMatrix = Matrixy4x4::Identity();

std::shared_ptr<btCylinderShape> Perviousity::Activators::StandButton::StaticMesh = nullptr;
std::shared_ptr<btSphereShape> Perviousity::Activators::StandButton::GhostShape = nullptr;

std::shared_ptr<btBoxShape> Perviousity::Activators::PressurePad::StaticShape = nullptr;
std::shared_ptr<btBoxShape> Perviousity::Activators::PressurePad::GhostShape = nullptr;

std::shared_ptr<btBoxShape> Perviousity::Shapes::CubeyBoi::CubeShape = nullptr;

//#define WALL_CALL Drawing::DrawCall{WallGeometryKey, 0ull, g_OpaqueMaterial, nullptr, true}
//#define FLOOR_CALL Drawing::DrawCall{FloorGeometryKey, 0ull, g_OpaqueMaterial, nullptr, true}


const Camera & Perviousity::Player::IControllerState::GetCamera() const
{
	return *m_control->Cam;
}

btVector3 Perviousity::Player::IControllerState::GetLook() const
{
	return m_control->m_Look;
}

btVector3 Perviousity::Player::IControllerState::GetHorizontalLook() const
{
	return m_control->m_LookHorizontal;
}

btVector3 Perviousity::Player::IControllerState::GetRight() const
{
	return m_control->m_Right;
}

btVector3 Perviousity::Player::IControllerState::GetHorizontalRight() const
{
	return m_control->m_RightHorizontal;
}

btVector3 Perviousity::Player::IControllerState::GetPosition() const
{
	return m_control->m_RigidBody->getWorldTransform().getOrigin();
}

G1::IGSpace * Perviousity::Player::IControllerState::GetGSpace() const
{
	return m_control->GetContainer();
}

btScalar Perviousity::Player::IControllerState::GetTargetUpdateInterval() const
{
	return btScalar(*m_control->mResources->TargetUpdateInterval);
}

bool Perviousity::Player::IControllerState::CanJump() const
{
	return m_control->CanJump();
}

bool Perviousity::Player::IControllerState::Crouching() const
{
	return m_control->m_Crouched;
}

void Perviousity::Player::IControllerState::ChangeState(std::unique_ptr<IControllerState> newstate)
{
	m_control->m_TargetState = std::move(newstate);
}

void Perviousity::Player::IControllerState::SetWalkDirection(const btVector3 & dir)
{
	m_control->m_RigidBody->setLinearVelocity(btVector3{ dir.x(), m_control->m_RigidBody->getLinearVelocity().y(), dir.z() });
}

void Perviousity::Player::IControllerState::Jump(const btVector3 & impulse)
{
	m_control->Jump(impulse);
}

void Perviousity::Player::IControllerState::Crouch()
{
	m_control->m_Crouched = true;
}

void Perviousity::Player::IControllerState::StandUp()
{
	m_control->m_Crouched = false;
}

void Perviousity::Player::IControllerState::SetCrouchState(bool state)
{
	m_control->m_Crouched = state;
	if (state)
		m_control->ApplyCrouchForce();
}

void Perviousity::Player::IControllerState::StampGround()
{
	m_control->StampOnGround();
}

void Perviousity::Player::IControllerState::DisableGravity()
{
	//m_control->setUp({ 0.f, 0.f, 0.f });
}

void Perviousity::Player::IControllerState::EnableGravity()
{
	//m_control->setUp({ 0.f, 1.f, 0.f });
}

bool Perviousity::Player::IControllerState::HasGravity()
{
	//return m_control->getUp() == btVector3{ 0.f, 1.f, 0.f };
	return true;
}

bool Perviousity::Player::WalkingState::Receive(Events::IEvent * e)
{
	return false;
}

bool Perviousity::Player::WalkingState::Receive(Event::KeyInput * key)
{
	if (key->KeyCode == SDLK_w)
	{
		if (key->State)
			m_forwardness = m_back ? 0.f : 1.f;
		else
			m_forwardness = m_back ? -1.f : 0.f;
		m_forward = key->State;
	}
	else if (key->KeyCode == SDLK_s)
	{
		if (key->State)
			m_forwardness = m_forward ? 0.f : -1.f;
		else
			m_forwardness = m_forward ? 1.f : 0.f;
		m_back = key->State;
	}
	else if (key->KeyCode == SDLK_a)
	{
		if (key->State)
			m_rightness = m_right ? 0.f : -1.f;
		else
			m_rightness = m_right ? 1.f : 0.f;
		m_left = key->State;
	}
	else if (key->KeyCode == SDLK_d)
	{
		if (key->State)
			m_rightness = m_left ? 0.f : 1.f;
		else
			m_rightness = m_left ? -1.f : 0.f;
		m_right = key->State;
	}
	else if (key->KeyCode == SDLK_SPACE)
	{
		if (key->State)
		{
			if (CanJump())
			{
				DINFO("Jumped");
				Jump({ 0.f, 4.f, 0.f });
			}
		}
	}
	else if (key->KeyCode == SDLK_LSHIFT)
	{
		m_sprint = key->State;
	}
	else if (key->KeyCode == SDLK_LCTRL)
	{
		SetCrouchState(key->State);
	}
	return false;
}

void Perviousity::Player::WalkingState::Reset()
{
	m_forwardness = m_rightness = 0.f;
	m_forward = m_back = m_right = m_left = m_sprint = false;
	// if (m_HoldCrouch)
	StandUp();
	SetWalkDirection({ 0.f, 0.f, 0.f });
}

void Perviousity::Player::WalkingState::BeforePhysics(btScalar dt)
{
	btVector3 walkdir = { 0.f, 0.f, 0.f };
	if (m_forwardness)
		walkdir += GetHorizontalLook() * m_forwardness;
	if (m_rightness)
		walkdir += GetHorizontalRight() * m_rightness;
	if (walkdir.length2() > SIMD_EPSILON * SIMD_EPSILON)
	{
		StampGround();
		walkdir.normalize();
	}
	SetWalkDirection((walkdir * m_TotalWalkSpeed + walkdir * (btScalar)m_sprint * m_SprintMagnitude));
}

void Perviousity::Player::WalkingState::AfterPhysics()
{
}

struct CeilingRayCallback : btCollisionWorld::ClosestRayResultCallback
{
protected:
	btCollisionObject *m_me;

	bool Hit = false;

public:
	CeilingRayCallback(const btVector3&	from, const btVector3& to, btCollisionObject *me, int collisiongroup = btBroadphaseProxy::DefaultFilter, int collisionmask = btBroadphaseProxy::AllFilter) : ClosestRayResultCallback(from, to), m_me(me) { m_collisionFilterGroup = collisiongroup; m_collisionFilterMask = collisionmask; }

	bool HitCeiling() { return Hit; }

	virtual btScalar addSingleResult(btCollisionWorld::LocalRayResult &rayResult, bool normalInWorldSpace)
	{
		if (Hit)
			return btScalar(0.0);

		if (rayResult.m_collisionObject == m_me)
			return btScalar(1.0);

		btVector3 hitnormal;
		if (normalInWorldSpace)
		{
			hitnormal = rayResult.m_hitNormalLocal;
		}
		else
		{
			hitnormal = rayResult.m_collisionObject->getWorldTransform().getBasis()*rayResult.m_hitNormalLocal;
		}

		btScalar dotDown = btVector3(0.f, -1.f, 0.f).dot(hitnormal);
		if (dotDown > 0.f)
		{
			Hit = true;
			return btScalar(0.0);
		}

		return ClosestRayResultCallback::addSingleResult(rayResult, normalInWorldSpace);
	}
};

bool Perviousity::Player::CeilingTest(Perviousity::Player::PerviousPlayer * control, Perviousity::Player::CeilTestStuff stuff)
{
	if (stuff.UseRay)
	{

	}
	else
	{
		DINFO("Non Ray test not finished doing ray test anyway");
	}

	auto weak = control->GetContainer()->GetPhysicsWorld();

	std::shared_ptr<btDynamicsWorld> world = weak.lock();
	if (world)
	{
		btVector3 from = ((btVector3)stuff.From) - btVector3{ 0.f, stuff.LowerAmount, 0.f };
		btVector3 to = from + btVector3{ 0.f, stuff.Height, 0.f };

		CeilingRayCallback callback{ from, to, control->m_RigidBody.get() };

		world->rayTest(from, to, callback);

		return callback.HitCeiling();
	}
	return false;
}


Perviousity::Shapes::PerviousWall::PerviousWall(G1::IShapeThings ting, PerviousThings ting2) : IShape(ting), FullResourceHolder(ting.Resources), m_Things(ting2), m_SpecialMats(), m_WasOn(!ting2.Activated)
{
	Matrixy4x4 phys_trans;
	btVector3 axis, phys_axis;
	axis = m_Things.Normal.cross(btVector3{ 1.f, 0.f, 0.f });
	phys_axis = m_Things.Normal.cross(btVector3{ 0.f, 0.f, -1.f });
	float fac = acosf(m_Things.Normal.x());
	float phys_fac = acosf(-m_Things.Normal.z());

	Matrixy4x4 Rot = Matrixy4x4::RotationAxisR((floaty3)axis, fac);
	phys_trans = Matrixy4x4::RotationAxisR((floaty3)phys_axis, phys_fac);

	EnsurePerviousResources(mResources);
	m_SpecialMats[0] = m_SpecialMats[1] = m_SpecialMats[2] = m_SpecialMats[3] = DefaultMaterial;
	
	G1I::PhysicsThing pting;
	pting.Dynamic = false;
	pting.Shape = WallShape;
	pting.CollisionGroup = PERVIOUS_WALL;
	pting.CollisionMask = PLAYER | DYNAMIC_PERVIOUS;
	pting.Trans = Matrixy4x4::MultiplyE(phys_trans, Matrixy4x4::Translate(m_Things.Transform.Transform(floaty3{ 0.f, 0.f, 0.f })));

	m_Things.Transform = Matrixy4x4::MultiplyE(Rot, m_Things.Transform);

	m_Outer = new G1I::RawRendyShape(ting.WithName(GetName() + "'s Outer"), Drawing::DrawCall{ m_OuterGeo, nullptr, g_OpaqueMaterial, &m_Things.Transform, true, GetName() });
	m_Parts[0] = new G1I::RawRendyShape(ting.WithName(GetName() + "'s Part1"), Drawing::DrawCall{ m_PartGeos[0], nullptr, g_OpaqueMaterial, &m_Things.Transform, true, GetName() });
	m_Parts[1] = new G1I::RawRendyShape(ting.WithName(GetName() + "'s Part1"), Drawing::DrawCall{ m_PartGeos[1], nullptr, g_OpaqueMaterial, &m_Things.Transform, true, GetName() });
	m_Parts[2] = new G1I::RawRendyShape(ting.WithName(GetName() + "'s Part1"), Drawing::DrawCall{ m_PartGeos[2], nullptr, g_OpaqueMaterial, &m_Things.Transform, true, GetName() });
	m_Parts[3] = new G1I::RawRendyShape(ting.WithName(GetName() + "'s Part1"), Drawing::DrawCall{ m_PartGeos[3], nullptr, g_OpaqueMaterial, &m_Things.Transform, true, GetName() });
	//m_Render = new G1I::RawRendyShape(ting.WithName(ting.Name + "'s Render Shape"), Drawing::DrawCall{ m_WallGeo, nullptr, g_OpaqueMaterial, &m_Things.Transform, true, GetName() });
	m_Wall = new G1I::PhysicsBody(ting.WithName(GetName() + "'s Physical Body"), pting);
	//m_Wall = new G1I::CubeyShape(Container, mResources, , G1I::Static, WallShape.get(), Matrixy4x4::MultiplyE(Rot, m_Things.Transform), PERVIOUS_WALL, PLAYER | DYNAMIC_PERVIOUS);

	if (m_Things.ExternalHolder)
		m_Wall->GetBody()->setUserPointer(m_Things.ExternalHolder);
	else
		m_Wall->GetBody()->setUserPointer(&m_Holder);

	if (!m_Things.Activated)
		TurnOff();

	//(void)this->AddChild(m_Render);
	(void)this->AddChild(m_Outer);
	(void)this->AddChild(m_Parts[0]);
	(void)this->AddChild(m_Parts[1]);
	(void)this->AddChild(m_Parts[2]);
	(void)this->AddChild(m_Parts[3]);
	(void)this->AddChild(m_Wall);
}

void Perviousity::Shapes::PerviousWall::TurnOff()
{
	m_Things.Activated = false;
}

void Perviousity::Shapes::PerviousWall::TurnOn()
{
	m_Things.Activated = true;
}

bool Perviousity::Shapes::PerviousWall::Enabled()
{
	return m_Things.Activated;
}

void Perviousity::Shapes::PerviousWall::AddMaterial(std::shared_ptr<PlatformMaterial> mat)
{
	if (m_SpecialMats[0] == DefaultMaterial)
	{
		// No materials have previous been assigned, fill up all 4 slots
		m_SpecialMats[0] = m_SpecialMats[1] = m_SpecialMats[2] = m_SpecialMats[3] = mat;
	}
	else if (m_SpecialMats[2] == m_SpecialMats[0])
	{
		// Only 1 Material has previous been assigned, fill up the last two slots
		m_SpecialMats[2] = m_SpecialMats[3] = mat;
	}
	else if (m_SpecialMats[3] == m_SpecialMats[2])
	{
		// 2 Materials have previously been assigned, take up the last slot
		m_SpecialMats[3] = mat;
	}
	else if (m_SpecialMats[1] == m_SpecialMats[0])
	{
		// 3 Materials have previously been assigned, take up the 2nd slot
		m_SpecialMats[1] = mat;
	}
	else
	{
		DWARNING("4 Materials have already been assigned to this: " + GetName());
	}
}

void Perviousity::Shapes::PerviousWall::MakeOff()
{
	m_Wall->GetBody()->setCollisionFlags(m_Wall->GetBody()->getCollisionFlags() | btCollisionObject::CF_NO_CONTACT_RESPONSE);
	m_Wall->GetBody()->getBroadphaseHandle()->m_collisionFilterGroup = DISABLED_PERVIOUS_WALL;
	//mResources->Ren3->GetDrawCall(m_Render->GetDrawCallKey())->Material = g_TransparentMaterial;
	//m_Render->GetCall()->Material = g_TransparentMaterial;
	m_Outer->GetCall()->Material = g_TransparentMaterial;
	for (int i = 0; i < 4; ++i)
	{
		m_Parts[i]->GetCall()->Material = m_SpecialMats[i]->TransparentVersion;
	}
}

void Perviousity::Shapes::PerviousWall::MakeOn()
{
	m_Wall->GetBody()->setCollisionFlags(m_Wall->GetBody()->getCollisionFlags() & (~btCollisionObject::CF_NO_CONTACT_RESPONSE));
	m_Wall->GetBody()->getBroadphaseHandle()->m_collisionFilterGroup = PERVIOUS_WALL;
	//mResources->Ren3->GetDrawCall(m_Wall->GetDrawCallKey())->Material = g_OpaqueMaterial;
	//m_Render->GetCall()->Material = g_OpaqueMaterial;
	m_Outer->GetCall()->Material = g_OpaqueMaterial;
	for (size_t i = 0; i < 4; ++i)
	{
		m_Parts[i]->GetCall()->Material = m_SpecialMats[i]->OpaqueVersion;
	}
}

Perviousity::Player::PerviousPlayer::PerviousPlayer(G1::IGSpace * container, CommonResources * resources, Perviousity::Player::PlayerStuff stuff)
	: IShape(container)
	, FullResourceHolder(resources)
	, Cam(new Camera())
	, m_FeetSource(Audio::CreateSauce())
	, m_FootstepSounds(GetFootstepSounds())
	, m_FootstepRandomizer(0ull, (m_FootstepSounds.empty() ? 0 : m_FootstepSounds.size() - 1))
	, m_Stuff(stuff)
{
	Name = "Puzzle Controller";
	Cam->SetLens(140.f * 0.5f * Math::DegToRadF, (float)*mResources->AspectRatio, IdealNearPlane, IdealFarPlane);
	Cam->SetPosLookUp(m_Stuff.InitialPosition, m_Stuff.Direction, { 0.f, 1.f, 0.f });
	Container->SetCamera(Cam);

	if (!Capsule)
		Capsule = std::make_unique<btCylinderShape>(btVector3{ btScalar(CapsuleRadius), btScalar(CapsuleHeight), btScalar(CapsuleRadius) });

	if (!SweepCapsule)
		SweepCapsule = std::make_unique<btCapsuleShape>(btScalar(CapsuleRadius + CapsuleRadiusBonus), btScalar(RayFinalLength - CapsuleRadius - CapsuleRadiusBonus));

	btVector3 LocalInertia;
	Capsule->calculateLocalInertia(Fatness, LocalInertia);

	m_RigidBody = std::make_shared<btRigidBody>(btRigidBody::btRigidBodyConstructionInfo(Fatness, this, Capsule.get(), LocalInertia));
	m_RigidBody->setAngularFactor(btScalar(0.0));
	m_RigidBody->setActivationState(DISABLE_DEACTIVATION);
	m_RigidBody->setFriction(0.f);
	m_RigidBody->setRestitution(0.f);
	m_RigidBody->setRollingFriction(0.f);
	m_RigidBody->setSpinningFriction(0.f);

	m_KinematicFeet = std::make_shared<btRigidBody>(btRigidBody::btRigidBodyConstructionInfo(0.f, &m_FeetState, SweepCapsule.get()));
	m_KinematicFeet->setCollisionFlags(m_KinematicFeet->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT);
	m_KinematicFeet->setActivationState(DISABLE_DEACTIVATION);

	Container->RequestPhysicsCall(m_RigidBody, PLAYER, ENVIRONMENT | DYNAMIC_PERVIOUS | DYNAMIC_PERVIOUS_INVERTED | PERVIOUS_WALL);
	Container->RequestPhysicsCall(m_KinematicFeet, PLAYER_FEET, PLAYER_FEET_DETECTOR);

	m_RigidBody->getWorldTransform().setOrigin(m_Stuff.InitialPosition);

	UpdateRayCache();

	m_PullTimer.Reset();
	m_PullTimer.Stop();

	mResources->Event->Add(this);
	mResources->Config->SafeSet({ "MouseSensitivityInverse" }, 1.f / 250.f);
}

Perviousity::Player::PerviousPlayer::~PerviousPlayer()
{
	UnlockMouse();
}

void Perviousity::Player::PerviousPlayer::BeforeDraw()
{
}

void Perviousity::Player::PerviousPlayer::AfterDraw()
{
	DoCam();

	TestForOutOfBounds();

	//DoPhys();

	DoSound();

	DoPickup();

	UpdateCache();

	if (m_TargetState)
		m_CurrentState = std::move(m_TargetState);

	if (!m_CurrentState)
		m_CurrentState = std::make_unique<WalkingState>(this);

	m_CurrentState->BeforePhysics(btScalar(*mResources->DeltaTime));
}

bool Perviousity::Player::PerviousPlayer::Receive(Events::IEvent * event)
{
	return false;
}

bool Perviousity::Player::PerviousPlayer::Receive(Event::WindowFocusEvent * e)
{
	if (!e->GainedFocus && m_MouseIsLocked)
		UnlockMouse();
	rightness = forwardness = 0.f;
	forward = back = right = left = false;
	
	if (m_CurrentState)
		m_CurrentState->Reset();

	return Events::RelevantEvent;
}

bool Perviousity::Player::PerviousPlayer::Receive(Event::AfterPhysicsEvent * e)
{
	PROFILE_PUSH("Pervious Player AFE");
	DoPhys();

	if (!m_IsSpirit)
	{
		floaty3 ass{ (m_RigidBody->getWorldTransform().getOrigin() + btVector3{ 0.f, 0.45f * CapsuleHeight, 0.f }) };
		Cam->SetPosition(ass);
		btTransform trans = m_RigidBody->getWorldTransform();
		trans.setOrigin(trans.getOrigin() - btVector3{ 0.f, m_ConvexOffset + RayFinalLength * (m_Crouched ? CrouchScale : 1.f), 0.f });
		m_FeetState.setWorldTransform(trans);
	}

	PROFILE_POP();
	return Events::RelevantEvent;
}

bool Perviousity::Player::PerviousPlayer::Receive(Event::KeyInput * key)
{
	if (m_IsSpirit)
	{
		if (key->KeyCode == SDLK_w)
		{
			if (key->State)
				forwardness = back ? 0.f : 1.f;
			else
				forwardness = back ? -1.f : 0.f;
			forward = key->State;
		}
		else if (key->KeyCode == SDLK_s)
		{
			if (key->State)
				forwardness = forward ? 0.f : -1.f;
			else
				forwardness = forward ? 1.f : 0.f;
			back = key->State;
		}
		else if (key->KeyCode == SDLK_a)
		{
			if (key->State)
				rightness = right ? 0.f : -1.f;
			else
				rightness = right ? 1.f : 0.f;
			left = key->State;
		}
		else if (key->KeyCode == SDLK_d)
		{
			if (key->State)
				rightness = left ? 0.f : 1.f;
			else
				rightness = left ? -1.f : 0.f;
			right = key->State;
		}
	}
	else
	{
		if (key->KeyCode == SDLK_e)
		{
			if (key->State)
				if (m_PickedUp)
					ResetPickup();
				else
					ShootInteractRay();
		}
	}
	if (key->KeyCode == SDLK_MINUS)
	{
		if (key->State)
		{
			if (!m_IsSpirit)
				SpiritMode();
			else
				NormalMode();
		}
	}
	if (key->KeyCode == SDLK_p)
	{
		if (key->State)
		{
			Requests::Request ass("ToggleDebugDraw");
			Container->Request(ass);
		}
	}
	else if (key->KeyCode == SDLK_o)
	{
		if (key->State)
		{
			Requests::Request ass("ToggleRealDraw");
			Container->Request(ass);
		}
	}
	else if (key->KeyCode == SDLK_EQUALS)
	{
		if (key->State)
		{
			Requests::Request ass("TogglePhysics");
			Container->Request(ass);
		}
	}
	else if (key->KeyCode == SDLK_LALT)
	{
		if (key->State)
		{
			UnlockMouse();
		}
	}
	else if (m_CurrentState && !m_IsSpirit)
		return m_CurrentState->Receive(key);
	return Events::RelevantEvent;
}

bool Perviousity::Player::PerviousPlayer::Receive(Event::MouseButton * e)
{
	if (e->Button == LMB && e->State)
	{
		if (m_MouseIsLocked)
		{
			ShootPewPewRay();
		}
		else
		{
			LockMouse();
		}
	}
	return Events::RelevantEvent;
}

bool Perviousity::Player::PerviousPlayer::Receive(Event::ResizeEvent * e)
{
	(void)e;
	Cam->SetLens(140.f * 0.5f * Math::DegToRadF, float(*mResources->AspectRatio), IdealNearPlane, IdealFarPlane);
	return Events::RelevantEvent;
}

void Perviousity::Player::PerviousPlayer::ShootPewPewRay()
{
	DINFO("Shooting pew pew ray");
	btVector3 from = Cam->GetPosition();
	btVector3 to = from + (Cam->GetLook() * ShootDistance);
	btCollisionWorld::ClosestRayResultCallback rayboi(from, to);
	rayboi.m_collisionFilterMask = PERVIOUS_WALL | DISABLED_PERVIOUS_WALL | ENVIRONMENT;
	rayboi.m_collisionFilterGroup = PLAYER;

	auto weakworld = Container->GetPhysicsWorld();

	std::shared_ptr<btDynamicsWorld> world = weakworld.lock();
	if (world)
	{
		world->rayTest(from, to, rayboi);

		if (rayboi.hasHit())
		{
			auto *hitman = rayboi.m_collisionObject;
			if (hitman->getUserPointer())
			{
				auto *point = reinterpret_cast<BulletHelp::NothingHolder*>(hitman->getUserPointer());
				if (point)
				{
					auto *thing = dynamic_cast<Shapes::IPerviousThing*>(point->Pointy);
					if (thing)
					{
						if (thing->AllowPlayerActivate())
						{
							if (thing->Enabled())
								thing->TurnOff();
							else
								thing->TurnOn();
						}
					}
					else
					{
						DINFO("Didn't hit a Pervious thing");
					}
				}
			}
		}
		else
		{
			DINFO("Didn't hit anything :(");
		}
	}
}

bool Perviousity::Player::PerviousPlayer::ShootInteractRay()
{
	btVector3 from = Cam->GetPosition();
	btVector3 to = from + (Cam->GetLook() * InteractDistance);
	btCollisionWorld::ClosestRayResultCallback rayboi(from, to);

	rayboi.m_collisionFilterMask = INTERACTABLE | ENVIRONMENT | PERVIOUS_WALL;
	rayboi.m_collisionFilterGroup = INTERACTION_RAY;

	auto weak = Container->GetPhysicsWorld();

	auto world = weak.lock();
	if (world)
	{
		world->rayTest(from, to, rayboi);

		if (rayboi.hasHit())
		{
			BulletHelp::NothingHolder *p = static_cast<BulletHelp::NothingHolder*>(rayboi.m_collisionObject->getUserPointer());
			if (p)
			{
				Activators::IActivatable *pp = dynamic_cast<Activators::IActivatable*>(p->Pointy);
				if (pp)
				{
					if (pp->AllowPlayerActivate())
						pp->Activate();

					return true;
				}
				else
				{
					Perviousity::Shapes::IPickupAble* pickup = dynamic_cast<Shapes::IPickupAble*>(p->Pointy);
					if (pickup)
					{
						for (auto &ontopof : m_PickupAblesOnTopOf)
							if (pickup == ontopof)
							{
								DINFO("You're too fat to pick yourself up"); // Easter egg
								return true;
							}
						m_PickedUp = pickup->GetMe();
						m_PickedUp->GetBody()->forceActivationState(DISABLE_DEACTIVATION);

						return true;
					}
				}
			}
		}
	}

	return false;
}

void Perviousity::Player::PerviousPlayer::SpiritMode()
{
	DINFO("Spiriting");
	m_IsSpirit = true;
}

void Perviousity::Player::PerviousPlayer::NormalMode()
{
	DINFO("Normaling");
	m_IsSpirit = false;
	m_RigidBody->setWorldTransform(btTransform(btQuaternion::getIdentity(), btVector3(Cam->GetPosition()) + btVector3(btScalar(0.0), btScalar(-0.45f * CapsuleHeight), btScalar(0.0))));
}

void Perviousity::Player::PerviousPlayer::Jump(const btVector3 & jump)
{
	DisableGroundPull();
	m_RigidBody->applyCentralImpulse(jump * Fatness);
}

bool Perviousity::Player::PerviousPlayer::CanJump()
{
	return m_OnGround;
}

void Perviousity::Player::PerviousPlayer::DoCam()
{
	if (m_MouseIsLocked)
	{
		auto ychange = mResources->MousePosition->y != *mResources->CenterY, xchange = mResources->MousePosition->x != *mResources->CenterX;
		float scale;
		try
		{
			scale = std::any_cast<float>(mResources->Config->Get({ "MouseSensitivityInverse" }));
		}
		catch (const Config::CouldNotFindException & e)
		{
			(void)e;
			scale = 1.f / 250.f;
		}
		if (ychange)
		{
			Cam->Pitch(-float(float(mResources->MousePosition->y - *mResources->CenterY) * scale));
		}

		if (xchange)
		{
			Cam->RotateY(-float(float(mResources->MousePosition->x - *mResources->CenterX) * scale));
		}

		if (xchange || ychange)
		{
			SDL_WarpMouseInWindow(mResources->Window, *mResources->CenterX, *mResources->CenterY);
		}
	}
}

void Perviousity::Player::PerviousPlayer::DoPhys()
{
	PROFILE_PUSH("DoPhys");
	if (m_IsSpirit)
	{
		floaty3 eye = Cam->GetPosition();
		floaty3 looky = Cam->GetLook();
		floaty3 righty = Cam->GetRight();
		if (forwardness)
			eye += looky * (10.f * forwardness * (float)*mResources->DeltaTime);
		if (rightness)
			eye += righty * (10.f * rightness * (float)*mResources->DeltaTime);
		Cam->SetPosition(eye);

		if (m_UseRayTest)
		{
			for (size_t i = Raycount; i-- > 0; )
			{
				btVector3 LegStart = m_RigidBody->getWorldTransform().getOrigin() + m_RayOffset[i];
				btVector3 DeadZoneStart = LegStart + btVector3{ 0.f, -LegLength, 0.f };
				btVector3 FeetStart = DeadZoneStart + btVector3{ 0.f, -DeadZoneLength, 0.f };
				btVector3 StubStart = FeetStart + btVector3{ 0.f, -FeetLength, 0.f };
				btVector3 StubEnd = StubStart + btVector3{ 0.f, -StubLength, 0.f };

				auto drawer = Container->GetPhysicsWorld().lock()->getDebugDrawer();
				if (!drawer)
					break;

				// Leg Line
				drawer->drawLine(LegStart, DeadZoneStart, { 0.5f, 0.8f, 0.5f });

				// DeadZone Line
				drawer->drawLine(DeadZoneStart, FeetStart, { 0.9f, 0.4f, 0.4f });

				// Feet Line
				drawer->drawLine(FeetStart, StubStart, { 0.5f, 0.8f, 0.5f });

				// Stub Line
				drawer->drawLine(StubStart, StubEnd, { 0.3f, 0.3f, 0.3f });
			}
		}
		else
		{
			auto drawer = Container->GetPhysicsWorld().lock()->getDebugDrawer();
			if (drawer)
			{
				btTransform trans = m_RigidBody->getWorldTransform();
				trans.setOrigin(trans.getOrigin() -	btVector3{ 0.f, m_ConvexOffset + RayFinalLength * (m_Crouched ? CrouchScale : 1.f), 0.f });
				drawer->drawCapsule(CapsuleRadius + CapsuleRadiusBonus, RayFinalLength * 0.5f, 1, trans, btVector3{ 0.8f, 0.4f, 0.7f });
			}
		}
	}
	else
	{
		DoFeetRays();
	}
	PROFILE_POP();	
}

void Perviousity::Player::PerviousPlayer::DoPickup()
{
	if (m_PickedUp)
	{
		for (auto &ontopof : m_PickupAblesOnTopOf)
			if (m_PickedUp.get() == ontopof)
			{
				DINFO("Oh very clever, pick it up, and *then* hop on it, too bad"); // Easter egg
				ResetPickup();
				return;
			}

		btVector3 TargetPoint = Cam->GetPosition();
		btVector3 look = Cam->GetLook();
		TargetPoint += look * m_PickupOffset;

		btVector3 PickupVelocity;
		btVector3 curPos = m_PickedUp->GetPosition();
		btVector3 dif = TargetPoint - curPos;
		if (dif.length2() > (m_MaxPickupDistance * m_MaxPickupDistance))
		{
			ResetPickup();
			return;
		}
		PickupVelocity = dif * s_PickupMoveFactor;
		
		// Determine which way to rotate
		btTransform trans = m_PickedUp->GetTransform();
		
		btTransform cam = Matrixy4x4::InvertedOrIdentity(Cam->View());

		float camx, camy, camz;
		cam.getRotation().getEulerZYX(camz, camy, camx);

		float px, py, pz;
		trans.getRotation().getEulerZYX(pz, py, px);

		/*btVector3 angvel = { Math::DifAngles(camx, px) * s_PickupSpinFactor, Math::DifAngles(camy, py) * s_PickupSpinFactor, Math::DifAngles(camz, pz) * s_PickupSpinFactor };
		float strength = angvel.length2();
		static float last_strength = 100.f;
		static float last_camx = 0.f, last_camy = 0.f, last_camz = 0.f, last_px = 0.f, last_py = 0.f, last_pz = 0.f;
		if (strength - 0.2f > last_strength)
			DINFO("Got em with current: (" + std::to_string(last_camx) + ", " + std::to_string(last_camy) + ", " + std::to_string(last_camz) + "), (" + std::to_string(last_px) + ", " + std::to_string(last_py) + ", " + std::to_string(last_pz) + ")");
		DINFO("Angular Velocity Strength: " + std::to_string(strength));
		last_strength = strength;
		last_camx = camx; last_camy = camy; last_camz = camz;
		last_px = px; last_py = py; last_pz = pz;*/

		btVector3 a, b;

		// Assign a and b
		a = { look.x(), 0.f, look.z() };
		a.safeNormalize();
		// Search for closest normal
		float closestDistance = 2.f;
		btVector3 normals[6];
		btVector3 &xpos = normals[0], &xneg = normals[1]
			, &ypos = normals[2], &yneg = normals[3]
			, &zpos = normals[4], &zneg = normals[5];
		xpos = trans.getBasis() * btVector3{ 1.f, 0.f, 0.f };
		xneg = xpos * -1.f;
		ypos = trans.getBasis() * btVector3{ 0.f, 1.f, 0.f };
		yneg = ypos * -1.f;
		zpos = trans.getBasis() * btVector3{ 0.f, 0.f, 1.f };
		zneg = zpos * -1.f;
		float tempDistance;

		for (size_t i = 0; i < 6; ++i)
		{
			tempDistance = (a - normals[i]).length2();
			if (tempDistance < closestDistance)
			{
				closestDistance = tempDistance;
				b = normals[i];
			}
		}

		btVector3 angvel;

		float angle = (acosf(b.dot(a)));
		if (angle > Math::DegToRadF * 5.f)
		{
			angvel = b.cross(a);
			angvel = angvel * (s_PickupSpinFactor * angle * angle);

			m_PickedUp->GetBody()->setAngularVelocity(angvel);
		}
		else
		{
			// Close enough, snap to, and zero out the velocity
			m_PickedUp->GetBody()->setAngularVelocity({ 0.f, 0.f, 0.f });
			angvel = b.cross(a) * angle;
			trans.setRotation(btQuaternion(b.y() + angvel.y(), b.x() + angvel.x(), b.z() + angvel.z()));
		}


		m_PickedUp->SetVelocity(m_RigidBody->getLinearVelocity() + PickupVelocity);
	}
}

void Perviousity::Player::PerviousPlayer::UpdateCache()
{
	auto look = Cam->GetLook();
	m_Look = look;
	m_LookHorizontal.setX(look.x);
	m_LookHorizontal.setZ(look.z);
	if (m_Look.getY() < (1.f - SIMD_EPSILON))
		m_LookHorizontal.normalize();

	auto right = Cam->GetRight();
	m_Right = right;
	m_RightHorizontal.setX(right.x);
	m_RightHorizontal.setZ(right.z);
	if (m_Right.getY() < (1.0 - SIMD_EPSILON))
		m_RightHorizontal.normalize();
}

void Perviousity::Player::PerviousPlayer::DoSound()
{
	// Update Listener
	floaty3 pos = Cam->GetPosition();
	floaty3 up = Cam->GetUp();
	floaty3 look = Cam->GetLook();
	float orientation[] = { look.x, look.y, look.z, up.x, up.y, up.z };
	btVector3 vel = m_RigidBody->getLinearVelocity();
	alListener3f(AL_POSITION, pos.x, pos.y, pos.z);
	alListener3f(AL_VELOCITY, vel.x(), vel.y(), vel.z());
	alListenerfv(AL_ORIENTATION, orientation);
}

void Perviousity::Player::PerviousPlayer::TestForOutOfBounds()
{
	// Test for out of bounds
	
	float out_of_bounds = DefaultOutOfBounds;
	if (auto plevel = std::any_cast<float>(mResources->Config->GetN(Config::Key{ "OutOfBoundsLevel" })))
		out_of_bounds = *plevel;

	if (Cam->GetPosition().y < out_of_bounds)
	{
		Cam->SetPosLookUp(m_Stuff.InitialPosition, m_Stuff.Direction, { 0.f, 1.f, 0.f });
		m_RigidBody->getWorldTransform().setOrigin(m_Stuff.InitialPosition);
		m_RigidBody->setLinearVelocity({ 0.f, 0.f, 0.f });
	}
}

void Perviousity::Player::PerviousPlayer::ResetPickup()
{
	if (m_PickedUp)
	{
		m_PickedUp->GetBody()->activate(true);
		m_PickedUp.reset();
	}
}

struct GroundNotMeCallback : btCollisionWorld::ClosestRayResultCallback
{
protected:
	btRigidBody *m_me = nullptr;
	btScalar m_minDot = btScalar(0.0);
	btVector3 m_up;
	std::vector<Perviousity::Shapes::IPickupAble*>& Pickupables;
public:
	GroundNotMeCallback(btRigidBody *me, std::vector<Perviousity::Shapes::IPickupAble*> &pickupables, btScalar minDot, const btVector3 &up, const btVector3 &from, const btVector3 &to) : ClosestRayResultCallback(from, to), m_me(me), m_minDot(minDot), m_up(up), Pickupables(pickupables) {}


	virtual btScalar addSingleResult(btCollisionWorld::LocalRayResult &rayresult, bool normalInWorld) override
	{
		if (rayresult.m_collisionObject == m_me)
		{
			return btScalar(1.0);
		}

		BulletHelp::NothingHolder* p = static_cast<BulletHelp::NothingHolder*>(rayresult.m_collisionObject->getUserPointer());
		if (p)
		{
			auto* pp = dynamic_cast<Perviousity::Shapes::IPickupAble*>(p->Pointy);
			if (pp)
			{
				Pickupables.emplace_back(pp);
			}
		}

		if (normalInWorld)
		{
			if (rayresult.m_hitNormalLocal.dot(m_up) < m_minDot)
				return btScalar(1.0);
		}
		else
			if (m_up.dot(rayresult.m_collisionObject->getWorldTransform().getBasis()*rayresult.m_hitNormalLocal) < m_minDot)
				return btScalar(1.0);

		return ClosestRayResultCallback::addSingleResult(rayresult, normalInWorld);
	}
};

struct GroundNotMeConvexCallbackRecordPickupables : btCollisionWorld::ClosestConvexResultCallback
{
protected:
	btRigidBody *m_me = nullptr;
	btScalar m_minDot = btScalar(0.0);
	btVector3 m_up;
	std::vector<Perviousity::Shapes::IPickupAble*>& Pickupables;
public:
	GroundNotMeConvexCallbackRecordPickupables(btRigidBody *me, std::vector<Perviousity::Shapes::IPickupAble*> &pickupables, btScalar minDot, const btVector3 &up, const btVector3 &from, const btVector3 &to) : ClosestConvexResultCallback(from, to), m_me(me), m_minDot(minDot), m_up(up), Pickupables(pickupables) {}
	

	virtual btScalar addSingleResult(btCollisionWorld::LocalConvexResult &convexresult, bool normalInWorld) override
	{
		if (convexresult.m_hitCollisionObject == m_me)
		{
			return btScalar(1.0);
		}

		BulletHelp::NothingHolder* p = static_cast<BulletHelp::NothingHolder*>(convexresult.m_hitCollisionObject->getUserPointer());
		if (p)
		{
			auto* pp = dynamic_cast<Perviousity::Shapes::IPickupAble*>(p->Pointy);
			if (pp)
			{
				Pickupables.emplace_back(pp);
			}
		}

		if (normalInWorld)
		{
			if (convexresult.m_hitNormalLocal.dot(m_up) < m_minDot)
				return btScalar(1.0);
		}
		else
			if (m_up.dot(convexresult.m_hitCollisionObject->getWorldTransform().getBasis()*convexresult.m_hitNormalLocal) < m_minDot)
				return btScalar(1.0);

		return ClosestConvexResultCallback::addSingleResult(convexresult, normalInWorld);
	}

};

void Perviousity::Player::PerviousPlayer::UpdateRayCache()
{
	// Re-generate cache for ray positions
	if (!Raycount)
		return;

	m_RayOffset.resize(Raycount);

	if (Raycount == 1)
	{
		m_RayOffset[0] = { 0.f, DefaultY, 0.f };
		return;
	}

	float increment = 360.f / Raycount;
	float angle = -(increment * 0.5f) + 0.f;
	float x, z;

	for (size_t i = 0; i < Raycount; ++i)
	{
		angle += increment;
		x = sinf(angle * Math::DegToRadF);
		z = cosf(angle * Math::DegToRadF);
		x *= CapsuleRadius * RayCircleScale;
		z *= CapsuleRadius * RayCircleScale;
		m_RayOffset[i] = { x, DefaultY, z };
	}
}

void Perviousity::Player::PerviousPlayer::DoFeetRays()
{
	PROFILE_PUSH("DoFeetRays");
	auto weak = Container->GetPhysicsWorld();
	
	auto world = weak.lock();

	if (!world)
	{
		DWARNING("Something gone wrong, could not lock the physics world");
		return;
	}
	
	bool wasonground = m_OnGround;

	m_OnGround = true;
	
	float determinant = GetDeterminant(world.get(), &m_DeepestNormal);

	PROFILE_PUSH("Post Determinant");
	if (determinant == -1.f)
	{
		m_OnGround = false;
		if (!wasonground && !m_DisableGroundPull)
			DisableGroundPull();
	}
	else
	{
		if (m_DisableGroundPull)
		{
			btVector3 vel = m_RigidBody->getLinearVelocity();
			float x = LegRayAmount - determinant;
			if (x > 0.f)// || (vel.y() - 0.05f) < 0.f)
			{
				EnableGroundPull();
				//if (vel.getY() > 0.f)
				//{
				//	DINFO("Applied Slow Spring Force (GroundPull disabled)");
				//	RigidBody->applyCentralForce(GetRotatedForce({ 0.f, SpringKSlowUp * x, 0.f }));
				//}
				//else
				//{
					// Spring math
					float springfac = GetSpringFactor(vel.getY());
					DINFO("Applied Spring Force (fac: " + std::to_string(springfac) + ", GroundPull disabled)");
					m_RigidBody->applyCentralForce(GetRotatedForce({ 0.f, SpringK * x * springfac, 0.f }));
				//}
			}
			else
			{
				DINFO("Determinant Beyond Spring Zone, GroundPull is disabled, and y velocity is upwards");
				m_OnGround = false;
			}
		}
		else
		{
			const auto &C = determinant;
			if (C > FeetRayStart && C < FeetRayEnd)
			{
				// Pull RigidBody down
				// Might as well simply move its transform (prevent bobbing (actually haven't tested if it prevents bobbing))

				float movedown = (C - FeetRayStart) * RayFinalLength;
				//if (movedown > MinPullDown)
				//{
					auto trans = m_RigidBody->getWorldTransform();
					btVector3 vel = m_RigidBody->getLinearVelocity();
					if (fabsf(vel.y()) > MinVelForMinPullDown)
					{
						movedown = fminf(MaxPullDown, movedown);
						DINFO("Pulled down Player by: " + std::to_string(movedown) + ", clipped");
					}
					else
						DINFO("Pulled down the Player by: " + std::to_string(movedown) + ", unclipped");
					trans.setOrigin(trans.getOrigin() + btVector3{ 0.f, -(movedown + 0.25f * DeadZoneLength), 0.f });
					m_RigidBody->setWorldTransform(trans);
					m_RigidBody->setLinearVelocity({ vel.x(), 0.f, vel.z() });
				//}
				//else
				//{
				//	DINFO("Did nothing as pulldown amount was too small (<" + std::to_string(MinPullDown));
				//}
			}
			else if (C < LegRayAmount)
			{
				// Push RigidBody up
				// For smoothness we'll use spring math

				// F = kx
				float x = Math::Map(C, 0.f, 1.f, LegRayAmount, 0.f);

				if (x < MinSpringX)
				{
					auto trans = m_RigidBody->getWorldTransform();
					trans.setOrigin(trans.getOrigin() - btVector3{ 0.f, x, 0.f });
					m_RigidBody->setWorldTransform(trans);
					DINFO("Did bounce fix");
				}
				else
				{
					//btVector3 vel = RigidBody->getLinearVelocity();
					//if (vel.getY() > 0.f)
					//{
					//	DINFO("Applied Slow Spring Force");
					//	RigidBody->applyCentralForce(GetRotatedForce({ 0.f, SpringKSlowUp * x, 0.f }));
					//}
					//else
					//{
					//	// Spring math
					//	DINFO("Applied Normal Spring Force");
					//	RigidBody->applyCentralForce(GetRotatedForce({ 0.f, SpringK * x, 0.f }));
					//}
					float springfac = GetSpringFactor(m_RigidBody->getLinearVelocity().getY());
					DINFO("Applied Spring Force (fac: " + std::to_string(springfac) + ")");
					m_RigidBody->applyCentralForce(GetRotatedForce({ 0.f, SpringK * x * springfac, 0.f }));
				}
			}
			else
			{
				static size_t LastDeadZoneUpdate = 0ull;
				static float DeadZonedDeterminant = 1.1f;
				if (LastDeadZoneUpdate != (*mResources->UpdateID - 1) || (determinant - 0.005f) > DeadZonedDeterminant || (determinant + 0.005f) < DeadZonedDeterminant)
				{
					DINFO("Dead Zoned with " + std::to_string(determinant));
					DeadZonedDeterminant = determinant;
				}
				LastDeadZoneUpdate = *mResources->UpdateID;
				// Dead zone(s)
			}
		}
	}
	PROFILE_POP();

	if (m_OnGround)
	{
		if (m_RigidBody->getGravity().length2() > (SIMD_EPSILON * SIMD_EPSILON))
		{
			DINFO("Disabling Gravity");
			m_RigidBody->setGravity({ 0.f, 0.f, 0.f });
			m_RigidBody->setFlags(m_RigidBody->getFlags() & (~BT_DISABLE_WORLD_GRAVITY));
		}
	}
	else
	{
		if (m_RigidBody->getGravity() != btVector3{ 0.f, -9.8f, 0.f })
		{
			DINFO("Re-enabling Gravity");
			m_RigidBody->setGravity({ 0.f, -9.8f, 0.f });
			m_RigidBody->setFlags(m_RigidBody->getFlags() | BT_DISABLE_WORLD_GRAVITY);
		}
	}
	PROFILE_POP();
}

btVector3 Perviousity::Player::PerviousPlayer::GetRotatedForce(btVector3 in)
{
	btVector3 norm = in;
	norm.safeNormalize();
	float dot = m_DeepestNormal.dot(in.normalized());
	if (dot < 0.f) // If dot is below zero the normal is facing down, which probably means there is no normal (and probably shouldn't be trying to rotate the force but eh)
		return in;

	float angle = acosf(dot);

	btVector3 rotaxis = m_DeepestNormal.cross(norm);
	btVector3 out = Matrixy4x4::RotationAxisR(floaty3{ rotaxis }, angle * RotateForceAmount).Transform(in);
	if (dot < 0.99619469809174553229501040247389f)
		DINFO("Input rotation: " + AsString(floaty3{ in }) + ", Norm: " + AsString(floaty3{ m_DeepestNormal }) + ", Rotated: " + AsString(floaty3{ out }));
	return out;
}

void Perviousity::Player::PerviousPlayer::DisableGroundPull()
{
	DINFO("Disabling Ground Pull");
	m_DisableGroundPull = true;
	m_PullTimer.Reset();
	m_PullTimer.Start();
}

void Perviousity::Player::PerviousPlayer::EnableGroundPull()
{
	m_PullTimer.Tick();
	if (m_PullTimer.TotalTime() > MinPullStart)
	{
		DINFO("Enabling Ground pull");
		m_DisableGroundPull = false;
	}
	else
	{
		DINFO("Too early to enable ground pull");
	}
}

void Perviousity::Player::PerviousPlayer::ApplyCrouchForce()
{
	m_RigidBody->applyCentralForce(btVector3{ 0.f, -CrouchForce * Fatness, 0.f });
}

void Perviousity::Player::PerviousPlayer::LockMouse()
{
	if (m_MouseIsLocked)
		return;

	int x, y;
	SDL_GetMouseState(&x, &y);
	mouse_locked_location = floaty2::TLtoGL({ (float)x, (float)y }, { *mResources->HalfWindowWidth, *mResources->HalfWindowHeight });
	SDL_WarpMouseInWindow(mResources->Window, *mResources->CenterX, *mResources->CenterY);
	SDL_ShowCursor(0);
	m_MouseIsLocked = true;
}

void Perviousity::Player::PerviousPlayer::UnlockMouse()
{
	if (!m_MouseIsLocked)
		return;

	SDL_ShowCursor(1);
	floaty2 TL = floaty2::GLtoTL(mouse_locked_location, *mResources->HalfWindowWidth, *mResources->HalfWindowHeight);
	SDL_WarpMouseInWindow(mResources->Window, (int)std::roundf(TL.x), (int)std::roundf(TL.y));
	m_MouseIsLocked = false;
}

void Perviousity::Player::PerviousPlayer::StampOnGround()
{
	if (!m_OnGround)
		return;
	if (m_FootstepSounds.empty())
		return;

	m_FootStepTimer.Start();
	m_FootStepTimer.Tick();
	double delta = m_FootStepTimer.TotalTime();
	if (delta > FootstepInterval)
	{
		floaty3 pos;
		alGetListener3f(AL_POSITION, &pos.x, &pos.y, &pos.z);
		m_FootStepTimer.Reset();
		// Update Source position
		m_FootstepSounds[m_FootstepRandomizer(*mResources->RandomGen)].AttachTo(m_FeetSource);
		m_FeetSource.SetPosition(pos.x, (m_Crouched ? pos.y - (0.95f * CapsuleHeight) - (0.5f * RayFinalLength) : pos.y - (0.95f * CapsuleHeight) - RayFinalLength), pos.z);
		m_FeetSource.Play();
	}
}

float Perviousity::Player::PerviousPlayer::GetDeterminant(btCollisionWorld *world, btVector3 *deepestnormal)
{
	PROFILE_PUSH("Getting Determinant");
	if (m_UseRayTest)
	{
		float running_sum = 0.f;
		size_t counted_rays = 0u;
		float closest = 1.f;

		for (size_t i = 0; i < Raycount; ++i)
		{
			btVector3 from = m_RigidBody->getWorldTransform().getOrigin() + m_RayOffset[i];
			btVector3 to = from + btVector3{ 0.f, -RayFinalLength * m_Crouched  ? CrouchScale : 1.f, 0.f };
			m_PickupAblesOnTopOf.clear();
			GroundNotMeCallback callback{ m_RigidBody.get(), m_PickupAblesOnTopOf, MinGroundDot, {0.f, 1.f, 0.f}, from, to };
			callback.m_collisionFilterGroup = PLAYER;
			callback.m_collisionFilterMask = PERVIOUS_WALL | ENVIRONMENT | DYNAMIC_PERVIOUS | DYNAMIC_PERVIOUS_INVERTED;

			world->getDebugDrawer()->drawLine(from, to, { 0.8f, 0.75f, 0.4f });
			world->rayTest(from, to, callback);
			
			// Only count ray if it hit
			if (callback.hasHit())
			{
				running_sum += callback.m_closestHitFraction;
				++counted_rays;
				if (callback.m_closestHitFraction < closest)
				{
					closest = callback.m_closestHitFraction;
					if (deepestnormal)
						*deepestnormal = callback.m_hitNormalWorld;
				}
			}
		}

		if (counted_rays)
			return running_sum / (float)counted_rays;
		else
			return -1.f;
	}
	else
	{
		// Do Convex Sweep Test

		btTransform from = m_RigidBody->getWorldTransform();
		from.setOrigin(from.getOrigin() - btVector3{ 0.f, m_ConvexOffset, 0.f });
		btTransform to = from;
		to.setOrigin(to.getOrigin() - btVector3{ 0.f, RayFinalLength * m_Crouched ? CrouchScale : 1.f, 0.f });
		btVector3 fromv = from.getOrigin();
		btVector3 tov = to.getOrigin();
		m_PickupAblesOnTopOf.clear();
		GroundNotMeConvexCallbackRecordPickupables callback{ m_RigidBody.get(), m_PickupAblesOnTopOf, MinGroundDot, {0.f, 1.f, 0.f}, fromv, tov };
		callback.m_collisionFilterGroup = PLAYER;
		callback.m_collisionFilterMask = PERVIOUS_WALL | ENVIRONMENT | DYNAMIC_PERVIOUS | DYNAMIC_PERVIOUS_INVERTED;

		world->getDebugDrawer()->drawCapsule(CapsuleRadius + CapsuleRadiusBonus, RayFinalLength * 0.5f, 1, to, btVector3{ 0.8f, 0.4f, 0.7f });
		world->convexSweepTest(SweepCapsule.get(), from, to, callback);
		
		if (callback.hasHit())
		{
			if (deepestnormal)
				*deepestnormal = callback.m_hitNormalWorld;
			return callback.m_closestHitFraction;
		}
		else
			return -1.f;
	}
	PROFILE_POP();
}

float Perviousity::Player::PerviousPlayer::GetSpringFactor(btScalar YVel)
{
	return fmaxf(0.01f, fminf(1.f, Math::Map(YVel, -2.f, 2.f, 1.f, 0.f)));
}

float Perviousity::Player::PerviousPlayer::GetNewYVelSpring(btScalar current, btScalar time)
{
	constexpr static float ReducedQuadStart = 0.f;
	constexpr static float LogStart = 2.f;

	// Three stages
	// Below Zero, treat as normal y = x^2
	// 2 > x > 0, treat as 0.5x^2
	// Beyond 2, treat as log(x) + 2 - log(2)

	float out = current;

	float excess = GetNewYStage1(current, time, out);

	if (excess)
		excess = GetNewYStage2(current, excess, out);

	if (excess)
		(void)GetNewYStage3(current, excess, out);

	return out;
}

float Perviousity::Player::PerviousPlayer::GetNewYStage1(btScalar current, btScalar time, btScalar & out)
{
	constexpr static float Stage2Start = 0.f;
	float current2 = current * current;
	float test = current2 * time;
	if (out + current > Stage2Start)
	{
		float allowed = Stage2Start - out;
		float allowedtime = allowed / current2;

		out += current2 * allowedtime;
		return time - allowedtime;
	}
	else
	{
		out += test;
		return 0.f;
	}
}

float Perviousity::Player::PerviousPlayer::GetNewYStage2(btScalar current, btScalar time, btScalar & out)
{
	constexpr static float Stage3Start = 2.f;
	float current2half = 0.5f * current * current;
	float test = current2half * time;
	if (out + test > Stage3Start)
	{
		float allowed = Stage3Start - out;
		float allowedtime = allowed / current2half;

		out += current2half * allowedtime;
		return time - allowedtime;
	}
	else
	{
		out += test;
		return 0.f;
	}
}

float Perviousity::Player::PerviousPlayer::GetNewYStage3(btScalar current, btScalar time, btScalar & out)
{
	//constexpr static float Stage4Start = SOMEBULLSHIT;

	out += log10f(current) * time;
	return 0.f;
}

std::vector<Audio::ALBufferI> Perviousity::Player::PerviousPlayer::GetFootstepSounds()
{
	std::vector<Audio::ALBufferI> sounds;
	try
	{
		std::error_code error;
		std::filesystem::directory_iterator footsteps{ mResources->WorkingDirectory + "Sounds/Footsteps", error };
		size_t count = 0;
		sounds.reserve(10ull); // Max 10 sounds
		if (error.value() == 0)
			for (const auto &p : footsteps)
			{
				if (count >= 10ull) // Max 10
					break;
				if (p.is_regular_file())
				{
					if (p.path().extension() == ".wav")
					{
						sounds.emplace_back(Audio::CreateBufferFile(p.path().generic_u8string()));
						++count;
					}
				}
			}
	}
	catch (const std::bad_alloc &e)
	{
		(void)e;
		DWARNING("Could not map Sounds/Footsteps/ directory");
	}
	return sounds;
}


namespace wtf
{

	struct datapack
	{
		btVector3 Normal;
		btVector3 Location;
		btScalar Depth;
	};
	// based on http://www.bulletphysics.org/mediawiki-1.5.8/index.php?title=Collision_Callbacks_and_Triggers#contactTest
	// Used to test if player character is on the ground
	struct GroundCallback : public btCollisionWorld::ContactResultCallback
	{
		GroundCallback(btRigidBody *tgtBody, btScalar mindot = 0.f) : btCollisionWorld::ContactResultCallback(), body(tgtBody), maxNormalDot(mindot) {}

		btRigidBody *body; // < The body the sensor is monitoring
		btScalar maxNormalDot;
		bool OnGround = false;
		btScalar foundDot = btScalar(0.0);
		datapack FoundData;
		std::vector<datapack> data;

		virtual bool needsCollision(btBroadphaseProxy* proxy) const {
			// superclass will check m_collisionFilterGroup and m_collisionFilterMask
			if (!btCollisionWorld::ContactResultCallback::needsCollision(proxy))
				return false;
			// if passed filters, may also want to avoid contacts between constraints
			return body && body->checkCollideWithOverride(static_cast<btCollisionObject*>(proxy->m_clientObject));
		}
		
		void GucciGang()
		{
			data.clear();
			FoundData.Depth = 0.f;
			FoundData.Location = FoundData.Normal = { 0.f, 0.f, 0.f };
			foundDot = 0.f;
			OnGround = false;
		}

		//! Called with each contact for your own processing (e.g. test if contacts fall in within sensor parameters)
		virtual btScalar addSingleResult(btManifoldPoint& cp,
			const btCollisionObjectWrapper* colObj0, int partId0, int index0,
			const btCollisionObjectWrapper* colObj1, int partId1, int index1)
		{
			if (colObj0->m_collisionObject == body)
			{
				data.push_back({ cp.m_normalWorldOnB, cp.getPositionWorldOnB(), cp.getDistance() });
			}
			else
			{
				assert(colObj1->m_collisionObject == body && "body does not match either collision object");
			}

			// do stuff with the collision point
			return 0; // There was a planned purpose for the return value of addSingleResult, but it is not used so you can ignore it.
		}

		void ProcessThatShit()
		{
			for (auto &dat : data)
			{
				float dot = dat.Normal.y();
				if (dot > foundDot)
				{
					foundDot = dot;
					FoundData = dat;
				}
			}
			if (foundDot > maxNormalDot)
				OnGround = true;
		}
	};
}

void Perviousity::Shapes::PerviousWall::CheckMaterials()
{
	bool broke = false;
	for (size_t i = 0; i < 4; ++i)
	{
		if (m_SpecialMats[i]->Lost)
		{
			broke = true;
			break;
		}
	}
	if (broke)
	{
		std::shared_ptr<PlatformMaterial> spare[3] = { DefaultMaterial, DefaultMaterial, DefaultMaterial };
		size_t spare_index = 0;
		for (size_t i = 0; i < 4; ++i)
		{
			if (!m_SpecialMats[i]->Lost) // Only re-add materials that aren't lost (there may be more than 1)
			{
				spare[spare_index] = m_SpecialMats[i];
				++spare_index;
			}
		}
		// Reset materials so I can re-add them
		m_SpecialMats[0] = m_SpecialMats[1] = m_SpecialMats[2] = m_SpecialMats[3] = DefaultMaterial;
		for (size_t i = 0; i < 3; ++i)
		{
			if (spare[i] != DefaultMaterial) // Don't re-add DefaultMaterial
				this->AddMaterial(spare[i]);
		}
	}
}

void Perviousity::Shapes::PerviousWall::EnsurePerviousResources(CommonResources * resources)
{
	if (!g_OpaqueMaterial)
		g_OpaqueMaterial = std::make_shared<struct Material>(OpaqueMaterialData);

	if (!g_TransparentMaterial)
		g_TransparentMaterial = std::make_shared<struct Material>(TransparentMaterialData);

	if (!DefaultMaterial)
	{
		DefaultMaterial = std::make_shared<PlatformMaterial>(OpaqueMaterialData);
		DefaultMaterial->TransparentVersion = g_TransparentMaterial;
	}

	if (!WallShape)
		WallShape = std::make_shared<btBoxShape>(btVector3(WallExtents.x, WallExtents.y, WallExtents.z));

	if (!FloorShape)
		FloorShape = std::make_unique<btBoxShape>(btVector3(FloorExtents.x, FloorExtents.y, FloorExtents.z));

	bool good = true;
	good &= (m_PartGeos[0] = s_PartGeos[0].lock()).operator bool();
	good &= (m_PartGeos[1] = s_PartGeos[1].lock()).operator bool();
	good &= (m_PartGeos[2] = s_PartGeos[2].lock()).operator bool();
	good &= (m_PartGeos[3] = s_PartGeos[3].lock()).operator bool();
	good &= (m_OuterGeo = s_OuterGeo.lock()).operator bool();
	good &= (m_DefaultMat = s_DefaultMat.lock()).operator bool();
	if (!good)
	{
		Assimp::Importer imp;
		PulledScene scene = PullScene(imp.ReadFile("platform.dae",
			aiProcess_ValidateDataStructure |
			aiProcess_GenSmoothNormals |
			aiProcess_CalcTangentSpace |
			aiProcess_Triangulate |
			aiProcess_JoinIdenticalVertices));

		if (scene.Nodes.empty())
		{
			DERROR("Could not find a valid platform.dae");
			return;
		}
		
		bool good = true;
		Damn::RawNode outer, _1, _2, _3, _4;

		good &= Damn::GetRawNodeFromScene(scene, "Outer", outer, mResources->Ren3);
		good &= Damn::GetRawNodeFromScene(scene, "Part1", _1, mResources->Ren3);
		good &= Damn::GetRawNodeFromScene(scene, "Part2", _2, mResources->Ren3);
		good &= Damn::GetRawNodeFromScene(scene, "Part3", _3, mResources->Ren3);
		good &= Damn::GetRawNodeFromScene(scene, "Part4", _4, mResources->Ren3);

		if (!good)
		{
			DERROR("Could not obtain raw nodes for outer and all 4 parts in platform.dae");
			return;
		}

		m_OuterGeo = outer.Geometry;
		s_OuterGeo = m_OuterGeo;

		m_PartGeos[0] = _1.Geometry;
		m_PartGeos[1] = _2.Geometry;
		m_PartGeos[2] = _3.Geometry;
		m_PartGeos[3] = _4.Geometry;
		s_PartGeos[0] = m_PartGeos[0];
		s_PartGeos[1] = m_PartGeos[1];
		s_PartGeos[2] = m_PartGeos[2];
		s_PartGeos[3] = m_PartGeos[3];

		m_DefaultMat = outer.Material;
		s_DefaultMat = m_DefaultMat;
	}
}
/*
Perviousity::Shapes::IChainedPerviousObject::IChainedPerviousObject(G1::IShapeThings shapedesc, floaty3 pos, PerviousFormType type, IChainedPerviousObject * link) 
	: IShape(shapedesc)
	, Link(link)
	, Position(pos)
	, Type(type)
{
	EnsurePerviousResources(shapedesc.Resources);

	if (Type < Floor) // Less than Floor means its a Wall
	{
		Body = new G1I::CubeyShape(shapedesc.Container, shapedesc.Resources, WALL_CALL, G1I::Static, WallShape.get(), Matrixy4x4::Translate(pos));
	}
	else // >= Floor means its a Floor type, or out of bounds (who cares if out of bounds)
	{
		Body = new G1I::CubeyShape(shapedesc.Container, shapedesc.Resources, FLOOR_CALL, G1I::Static, FloorShape.get(), Matrixy4x4::Translate(pos));
	}

	this->AddChild(Body); // Captures pointer in vector where it will be destroyed properly

	Body->SetUserPointer(&m_Holder);
}

void Perviousity::Shapes::IChainedPerviousObject::SetLink(IChainedPerviousObject * link)
{
	Link = link;
}

void Perviousity::Shapes::IChainedPerviousObject::TurnOn()
{
	if (Enabled())
		return;

	Body->GetBody()->setCollisionFlags(Body->GetBody()->getCollisionFlags() ^ btCollisionObject::CF_NO_CONTACT_RESPONSE);
	mResources->Ren3->GetDrawCall(Body->GetDrawCallKey())->Material = g_OpaqueMaterial;

	if (Link)
		Link->DoAction(true);
}

void Perviousity::Shapes::IChainedPerviousObject::TurnOff()
{
	if (!Enabled())
		return;

	Body->GetBody()->setCollisionFlags(Body->GetBody()->getCollisionFlags() | btCollisionObject::CF_NO_CONTACT_RESPONSE);
	mResources->Ren3->GetDrawCall(Body->GetDrawCallKey())->Material = g_TransparentMaterial;

	if (Link)
		Link->DoAction(false);
}

bool Perviousity::Shapes::IChainedPerviousObject::Enabled()
{
	return (~(Body->GetBody()->getCollisionFlags())) & btCollisionObject::CF_NO_CONTACT_RESPONSE;
}*/

Perviousity::Shapes::PairedPerviousObject::PairedPerviousObject(G1::IShapeThings ting, PairedThings ass, bool activated) : FullResourceHolder(ting.Resources), IShape(ting), Info(ass)
{
	m_Wall = new PerviousWall(ting.WithName(ting.Name + "'s Wall"), {ass.Trans, ass.Normal, activated, &m_Holder});
	
	this->AddChild(m_Wall);
}

void Perviousity::Shapes::PairedPerviousObject::SetPair(Pointer::f_ptr<PairedPerviousObject> p)
{ 
	Info.Pair = p; 
	if (Info.Copy)
	{
		if ((Enabled()) != (Info.Pair->Enabled()))
		{
			if (Enabled())
				Info.Pair->SecretTurnOn();
			else
				Info.Pair->SecretTurnOff();
		}
	}
	else
	{
		if ((Enabled()) == (Info.Pair->Enabled()))
		{
			if (Enabled())
				Info.Pair->SecretTurnOff();
			else
				Info.Pair->SecretTurnOn();
		}
	}
}

void Perviousity::Shapes::PairedPerviousObject::BeforeDraw()
{
}

void Perviousity::Shapes::PairedPerviousObject::AfterDraw()
{
	if (m_PairLastUpdate == *mResources->UpdateID)
	{
		if ((Info.Pair->Enabled() != Enabled()) == Info.Copy)
			m_Wall->KeepActivated(m_PairLastUpdate);
		/*
		Basically this does this:
		----------------------------
		|pairenabled|enabled| copy | output |
		----------------------------
		| 0			| 0		|    0    | 1   |
		| 0			| 0		|    1    | 0   |
		| 0			| 1		|    0    | 0   |
		| 0			| 1		|    1    | 1   |
		| 1			| 0		|    0    | 0   |
		| 1			| 0		|    1    | 1   |
		| 1			| 1		|    0    | 1   |
		| 1			| 1		|    1    | 0   |
		*/
	}
}

void Perviousity::Shapes::PairedPerviousObject::TurnOn()
{
	m_Wall->TurnOn();

	if (Info.Pair)
	{
		if (Info.Copy)
			Info.Pair->SecretTurnOn();
		else
			Info.Pair->SecretTurnOff();
	}
}

void Perviousity::Shapes::PairedPerviousObject::TurnOff()
{
	m_Wall->TurnOff();

	if (Info.Pair)
	{
		if (Info.Copy)
			Info.Pair->SecretTurnOff();
		else
			Info.Pair->SecretTurnOn();
	}
}

bool Perviousity::Shapes::PairedPerviousObject::Enabled()
{
	return (Info.LastActivateID == *mResources->UpdateID ? !m_Wall->Enabled() : m_Wall->Enabled());
}

void Perviousity::Shapes::PairedPerviousObject::DisablePlayerActivation()
{
	Info.NoActivate = true;
}

void Perviousity::Shapes::PairedPerviousObject::EnablePlayerActivation()
{
	Info.NoActivate = false;
}

void Perviousity::Shapes::PairedPerviousObject::KeepActivated(size_t updateid)
{
	// Check if the pair is setting this updateid
	if (m_PairLastUpdate == *mResources->UpdateID)
		return;

	m_Wall->KeepActivated(updateid);
	Info.LastActivateID = updateid;
	Info.Pair->m_PairLastUpdate = updateid;
}

bool Perviousity::Shapes::PairedPerviousObject::AllowPlayerActivate() const
{
	return !Info.NoActivate;
}

void Perviousity::Shapes::PairedPerviousObject::AddMaterial(std::shared_ptr<PlatformMaterial> mat)
{
	m_Wall->AddMaterial(mat);
}

void Perviousity::Shapes::PairedPerviousObject::SecretTurnOff()
{
	m_Wall->TurnOff();
}

void Perviousity::Shapes::PairedPerviousObject::SecretTurnOn()
{
	m_Wall->TurnOn();
}

Perviousity::Shapes::GroupPerviousObject::GroupPerviousObject(G1::IShapeThings ting, Perviousity::Shapes::GroupObjectInfoOld info) : GroupPerviousObject(ting, (GroupObjectInfo)info) {}

Perviousity::Shapes::GroupPerviousObject::GroupPerviousObject(G1::IShapeThings ting, GroupObjectInfo info, bool activated) : IShape(ting), FullResourceHolder(ting.Resources), Info(info)
{
	m_Wall = new Perviousity::Shapes::PerviousWall(ting.WithName("Group-owned Pervious Wall"), { info.Trans, info.Normal, activated, &m_Holder });

	this->AddChild(m_Wall);

	if (Info.Control)
	{
		if (activated)
			Info.Control->TryTurnOn(this);
		else
			Info.Control->TryTurnOff(this);
	}
}

void Perviousity::Shapes::GroupPerviousObject::TurnOn()
{
	if (Enabled())
		return;

	if (Info.Control)
	{
		Info.Control->TryTurnOn(this);
	}
}

void Perviousity::Shapes::GroupPerviousObject::TurnOff()
{
	if (!Enabled())
		return;

	if (Info.Control)
	{
		Info.Control->TryTurnOff(this);
	}
}

bool Perviousity::Shapes::GroupPerviousObject::Enabled()
{
	return m_Wall->Enabled();
}

void Perviousity::Shapes::GroupPerviousObject::AddMaterial(std::shared_ptr<PlatformMaterial> mat)
{
	m_Wall->AddMaterial(mat);
}

Perviousity::Shapes::GroupController::GroupController(G1::IShapeThings ting, GroupControllerInfo info) : FullResourceHolder(ting.Resources), IShape(ting), Info(info)
{
}

void Perviousity::Shapes::GroupController::TryTurnOff(GroupPerviousObject * obj)
{
	if (Info.DisableAbility)
	{
		if (!Info.Inverted)
		{
			if (InTargets(obj) != Targets.end())
				return;
			else
			{
				if (Targets.size() >= Info.PlatformCount)
				{
					Targets.front()->m_Wall->TurnOn();
					Targets.pop_front();
				}
				Targets.emplace_back(obj);
				obj->m_Wall->TurnOff();
			}
		}
		else
		{
			obj->m_Wall->TurnOff();
			auto it = InTargets(obj);
			if (it != Targets.end())
			{
				Targets.erase(it);
			}
		}
	}
}

void Perviousity::Shapes::GroupController::TryTurnOn(GroupPerviousObject * obj)
{
	if (Info.EnableAbility)
	{
		if (Info.Inverted)
		{
			if (InTargets(obj) != Targets.end())
				return;
			else
			{
				if (Targets.size() >= Info.PlatformCount)
				{
					Targets.front()->m_Wall->TurnOff();
					Targets.pop_front();
				}
				Targets.emplace_back(obj);
				obj->m_Wall->TurnOn();
			}
		}
		else
		{
			obj->m_Wall->TurnOn();
			auto it = InTargets(obj);
			if (it != Targets.end())
			{
				Targets.erase(it);
			}
		}
	}
}

void Perviousity::Shapes::GroupController::SetDisableAbility(bool val)
{
	Info.DisableAbility = val;
	DINFO(GetName() + " has lost it's ability to disable platforms");
	if (!val && !Info.EnableAbility)
	{
		DINFO(GetName() + " has effectively become a static set of platforms, platforms can neither be enabled or disabled");
	}
}

void Perviousity::Shapes::GroupController::SetEnableAbility(bool val)
{
	Info.EnableAbility = val;
	DINFO(GetName() + " has lost it's ability to enable platforms");
	if (!val && !Info.DisableAbility)
	{
		DINFO(GetName() + " has effectively become a static set of platforms, platforms can neither be enabled or disabled");
	}
}

std::list<Perviousity::Shapes::GroupPerviousObject*>::iterator Perviousity::Shapes::GroupController::InTargets(GroupPerviousObject * obj)
{
	auto it = Targets.begin();
	for (; it != Targets.end(); ++it)
		if (*it == obj)
			return it;
	return it;
}

Perviousity::Activators::StandButton::StandButton(G1::IShapeThings ting, StandButtonThings ting2) 
	: FullResourceHolder(ting.Resources)
	, IShape(ting)
	, m_Things(ting2.RemakeWithNewMatrix())
	, BodyState(GetBodyState())
	, m_SoundSource(Audio::CreateSauce())
{
	if (!m_Things.Activatable)
		DWARNING("Given activatable was null");
	else
	{
		if (m_Things.Is_End)
		{
			m_SpecialColor = std::make_shared<Perviousity::Shapes::PlatformMaterial>(Material{ floaty4{ 0.f, 0.f, 0.f, 0.f }, floaty4{ 0.8f, 0.2f, 0.2f, 1.f }, floaty4{ 0.f, 0.f, 0.4f, 1.f }, floaty4{ 0.f, 0.f, 0.4f, 10.f }, 1.f, 0 });
		}
		else
		{
			// Assign new Material
			m_SpecialColor = GetActivatorMaterial();
			auto *perv = dynamic_cast<Shapes::IPerviousThing *>(m_Things.Activatable.get());
			if (perv)
				perv->AddMaterial(m_SpecialColor);
		}
	}

	EnsureGeometry(ting2.Transform);

	(void)this->AddChild(new G1I::RawRendyShape({ GetContainer(), GetResources(), "ButtonRed" }, Drawing::DrawCall{ m_RedGeo, 0, m_RedMat, &this->RedWorld, true, GetName() + " Red Part" }));
	(void)this->AddChild(new G1I::RawRendyShape({ GetContainer(), GetResources(), "ButtonCase" }, Drawing::DrawCall{ m_CaseGeo, 0, m_CaseMat, &this->CaseWorld, true, GetName() + " Case Part" }));
	(void)this->AddChild(new G1I::RawRendyShape({ GetContainer(), GetResources(), "ButtonSpecial" }, Drawing::DrawCall{ m_SpecialGeo, 0, m_SpecialColor->OpaqueVersion, &this->StandWorld, true, GetName() + " Special Part" })); // Assume Special copies Stand's matrix
	(void)this->AddChild(new G1I::RawRendyShape({ GetContainer(), GetResources(), "ButtonStand" }, Drawing::DrawCall{ m_StandGeo, 0, m_StandMat, &this->StandWorld, true, GetName() + " Stand Part" }));

	EnsureShapes();

	this->StaticBody = std::make_shared<btRigidBody>(btRigidBody::btRigidBodyConstructionInfo(0.f, &BodyState, StaticMesh.get()));
	this->ButtonGhost = std::make_shared<btGhostObject>();
	this->ButtonGhost->setWorldTransform(Matrixy4x4::MultiplyE(m_Things.Transform, Matrixy4x4::Translate(0.f, 1.12f, 0.f)));
	ButtonGhost->setCollisionShape(GhostShape.get());
	ButtonGhost->setUserPointer(&m_Holder);

	Container->RequestPhysicsCall(StaticBody, ENVIRONMENT, PLAYER | DYNAMIC_PERVIOUS | DYNAMIC_PERVIOUS_INVERTED);
	Container->RequestPhysicsCall(ButtonGhost, ACTIVATOR_BUTTON | INTERACTABLE, INTERACTION_RAY);

	btVector3 ghostorigin = ButtonGhost->getWorldTransform().getOrigin();
	m_SoundSource.SetPosition(ghostorigin.x(), ghostorigin.y(), ghostorigin.z());

	// Ensure Sound
	if (!mResources->Config->HasType(Config::KeyC{ "ButtonSound" }, typeid(Audio::ALBufferI)))
	{
		mResources->Config->SafeSet(Config::KeyC{ "ButtonSound" }, Audio::CreateBufferFile("buttonpress.wav")); // Audio File Reference
	}
}

void Perviousity::Activators::StandButton::Activate()
{
	if (m_Things.Activatable)
	{
		m_Things.Activatable->Activate();

		// Play the sound
		Config::ExclusiveVariableLock lock = mResources->Config->GetExclusive(Config::KeyC{ "ButtonSound" });
		if (lock.Alive())
		{
			Audio::ALBufferI *buf = std::any_cast<Audio::ALBufferI>(&lock.operator std::any & ());
			if (buf)
			{
				buf->AttachTo(m_SoundSource);
				m_SoundSource.Play();
			}
		}
	}
}

void Perviousity::Activators::StandButton::EnsureGeometry(Matrixy4x4 trans)
{
	m_StandGeo = s_StandGeo.lock();
	m_StandMat = s_StandMat.lock();
	m_RedGeo = s_RedGeo.lock();
	m_RedMat = s_RedMat.lock();
	m_CaseGeo = s_CaseGeo.lock();
	m_CaseMat = s_CaseMat.lock();
	m_SpecialGeo = s_SpecialGeo.lock();
	if (!m_StandGeo || !m_StandMat ||
		!m_RedGeo || !m_RedMat ||
		!m_CaseGeo || !m_CaseMat ||
		!m_SpecialGeo)
	{
		// Import the gucci
		Assimp::Importer imp;
		PulledScene scene = PullScene(imp.ReadFile("button.dae", aiProcess_ValidateDataStructure |
			aiProcess_GenSmoothNormals |
			aiProcess_CalcTangentSpace |
			aiProcess_Triangulate |
			aiProcess_JoinIdenticalVertices));

		if (scene.Nodes.empty())
			throw EnsureFailure();

		std::unordered_multiset<PulledNode, PulledNodeHash, PulledNodeEqual>::iterator rit = scene.Nodes.find({ "ButtonRed" });
		if (rit == scene.Nodes.end())
			throw EnsureFailure();
		RedMatrix = Matrixy4x4::MultiplyE(rit->LocalTransform, scene.RootTransform);

		std::unordered_multiset<PulledNode, PulledNodeHash, PulledNodeEqual>::iterator cit = scene.Nodes.find({ "ButtonCase" });
		if (cit == scene.Nodes.end())
			throw EnsureFailure();
		CaseMatrix = Matrixy4x4::MultiplyE(cit->LocalTransform, scene.RootTransform);

		std::unordered_multiset<PulledNode, PulledNodeHash, PulledNodeEqual>::iterator sit = scene.Nodes.find({ "ButtonStand" });
		if (sit == scene.Nodes.end())
			throw EnsureFailure();
		StandMatrix = Matrixy4x4::MultiplyE(sit->LocalTransform, scene.RootTransform);

		Damn::RawNode StandRaw, RedRaw, CaseRaw, SpecialRaw;
		if (!Damn::GetRawNodeFromScene(scene, "ButtonRed", RedRaw, mResources->Ren3))
			throw EnsureFailure();

		if (!Damn::GetRawNodeFromScene(scene, "ButtonCase", CaseRaw, mResources->Ren3))
			throw EnsureFailure();

		if (!Damn::GetRawNodeFromScene(scene, "ButtonStand", StandRaw, mResources->Ren3))
			throw EnsureFailure();

		if (!Damn::GetRawNodeFromScene(scene, "ButtonSpecial", SpecialRaw, mResources->Ren3))
			throw EnsureFailure();

		m_StandMat = StandRaw.Material;
		m_StandGeo = StandRaw.Geometry;
		s_StandGeo = m_StandGeo;
		s_StandMat = m_StandMat;
		
		//m_CaseMat = CaseRaw.Material;
		m_CaseMat = std::make_shared<Material>(CaseConstMaterial);
		m_CaseGeo = CaseRaw.Geometry;
		s_CaseGeo = m_CaseGeo;
		s_CaseMat = m_CaseMat;

		//m_RedMat = RedRaw.Material;
		m_RedMat = std::make_shared<Material>(RedConstMaterial);
		m_RedGeo = RedRaw.Geometry;
		s_RedGeo = m_RedGeo;
		s_RedMat = m_RedMat;

		// Assume Special copies Stand's matrix

		m_SpecialGeo = SpecialRaw.Geometry;
		s_SpecialGeo = m_SpecialGeo;
	}
	this->RedWorld = Matrixy4x4::MultiplyE(RedMatrix, trans);
	this->CaseWorld = Matrixy4x4::MultiplyE(CaseMatrix, trans);
	this->StandWorld = Matrixy4x4::MultiplyE(StandMatrix, trans);
}

void Perviousity::Activators::StandButton::EnsureShapes()
{
	if (!StaticMesh)
		StaticMesh = std::make_shared<btCylinderShape>(btVector3{0.1f, 1.04246f * 0.5f, 0.1f});

	if (!GhostShape)
		GhostShape = std::make_shared<btSphereShape>(0.2f);
}

Matrixy4x4 Perviousity::Activators::StandButton::GetBodyState()
{
	Matrixy4x4 out;
	
	out = Matrixy4x4::Multiply(m_Things.Transform, Matrixy4x4::Translate({ 0.f, 0.5f * 1.04246f, 0.f }));

	return out;
}

void Perviousity::Shapes::IPerviousThing::Activate()
{
	if (this->Enabled())
		this->TurnOff();
	else
		this->TurnOn();
}

void Perviousity::Shapes::ActivateOnlyWall::ActualTurnOff()
{
	PerviousWall::TurnOff();
}

void Perviousity::Shapes::ActivateOnlyWall::ActualTurnOn()
{
	PerviousWall::TurnOn();
}

Perviousity::Shapes::ActivateOnlyWall::ActivateOnlyWall(G1::IShapeThings ting, PerviousThings ting2) : IShape(ting), FullResourceHolder(ting.Resources), PerviousWall(ting, ting2)
{
}

void Perviousity::Shapes::ActivateOnlyWall::TurnOff()
{
	// Do nothing
}

void Perviousity::Shapes::ActivateOnlyWall::TurnOn()
{
	// Do nothing
}

void Perviousity::Shapes::ActivateOnlyWall::Activate()
{
	if (Enabled())
		this->ActualTurnOff();
	else
		this->ActualTurnOn();
}

Perviousity::Activators::StandButtonThings &Perviousity::Activators::StandButtonThings::RemakeWithNewMatrix()
{
	// Make a new matrix with the origin and normal

	floaty3 translate = Transform.Transform({ 0.f, 0.f, 0.f });

	floaty3 rotaxis = floaty3(Normal.cross({ 0.f, 1.f, 0.f }));

	float rads = acosf(Normal.y());

	Transform = Matrixy4x4::MultiplyE(Matrixy4x4::RotationAxisR(rotaxis, rads), Matrixy4x4::Translate(translate));
	
	return *this;
}

Perviousity::Activators::PressurePad::PressurePad(G1::IShapeThings ting, PressurePadThings ting2) : FullResourceHolder(ting.Resources), IShape(ting), m_PadThings(ting2)
{
	if (!m_PadThings.Activatable)
		DINFO("Creating Dud Pad");

	m_SpecialMat = GetActivatorMaterial();

	auto *perv = dynamic_cast<Perviousity::Shapes::IPerviousThing *>(m_PadThings.Activatable.get());
	if (perv)
	{
		perv->AddMaterial(m_SpecialMat);
	}

	EnsureGeometry();

	m_PadThings.Transform;

	this->AddChild(new G1I::RawRendyShape({ GetContainer(), GetResources(), "PadBase" }, Drawing::DrawCall{ m_BaseGeo, 0, m_BaseMat, &m_PadThings.Transform, true }));
	this->AddChild(new G1I::RawRendyShape({ GetContainer(), GetResources(), "PadPressure" }, Drawing::DrawCall{ m_PressureGeo, 0, m_PressureMat, &m_PadThings.Transform, true }));
	this->AddChild(new G1I::RawRendyShape(ting.WithName(GetName() + "'s PadSpecial"), Drawing::DrawCall{ m_SpecialGeo, 0, m_SpecialMat->OpaqueVersion, &m_PadThings.Transform, true }));

	EnsureShapes();

	floaty3 Origin = m_PadThings.Transform.Transform({ 0.f, 0.f, 0.f });
	this->StaticBody = std::make_shared<btRigidBody>(btRigidBody::btRigidBodyConstructionInfo(0.f, nullptr, StaticShape.get()));
	StaticBody->setWorldTransform(Matrixy4x4::Translate(Origin + floaty3{ 0.f, 0.05f, 0.f }));
	this->PadGhost = std::make_shared<btGhostObject>();
	PadGhost->setCollisionShape(GhostShape.get());
	PadGhost->setUserPointer(&m_Holder);
	PadGhost->setWorldTransform(Matrixy4x4::Translate(Origin + floaty3{ 0.f, 0.09683f, 0.f }));

	Container->RequestPhysicsCall(StaticBody, ENVIRONMENT, PLAYER | DYNAMIC_PERVIOUS | DYNAMIC_PERVIOUS_INVERTED);
	Container->RequestPhysicsCall(PadGhost, ACTIVATOR_PAD | PLAYER_FEET_DETECTOR, PAD_PRESSURER | PLAYER_FEET);
}

void Perviousity::Activators::PressurePad::BeforeDraw()
{
	// Determine if there is a object colliding with the ghost
	
	if (!m_PadThings.Activatable)
		return;

	for (int i = PadGhost->getNumOverlappingObjects(); i-- > 0; )
	{
		auto *obj = PadGhost->getOverlappingObject(i);

		if (obj && obj->getBroadphaseHandle())
		{
			if (((obj->getBroadphaseHandle()->m_collisionFilterGroup & PAD_PRESSURER) && (obj->getBroadphaseHandle()->m_collisionFilterMask & ACTIVATOR_PAD)))
			{
				m_PadThings.Activatable->KeepActivated(*mResources->UpdateID);
				// Detected an Activator
				return;
			}
			if ((obj->getBroadphaseHandle()->m_collisionFilterGroup & PLAYER_FEET) && (obj->getBroadphaseHandle()->m_collisionFilterMask & PLAYER_FEET_DETECTOR))
			{
				m_PadThings.Activatable->KeepActivated(*mResources->UpdateID);
				// Detected The Player
				return;
			}
		}
	}
}

void Perviousity::Activators::PressurePad::AfterDraw()
{
}

void Perviousity::Activators::PressurePad::Activate()
{
}

void Perviousity::Activators::PressurePad::EnsureGeometry()
{
	m_BaseGeo = s_BaseGeo.lock();
	m_BaseMat = s_BaseMat.lock();
	m_SpecialGeo = s_SpecialGeo.lock();
	m_PressureGeo = s_PressureGeo.lock();
	m_PressureMat = s_PressureMat.lock();
	if (!m_BaseGeo || !m_BaseMat ||
		!m_PressureGeo || !m_PressureMat ||
		!m_SpecialGeo)
	{
		Assimp::Importer imp;
		PulledScene scene = PullScene(imp.ReadFile("pressurepad.dae", aiProcess_ValidateDataStructure |
			aiProcess_GenSmoothNormals |
			aiProcess_CalcTangentSpace |
			aiProcess_Triangulate));

		if (scene.Nodes.empty())
		{
			throw PullException();
		}

		// No normal use yet, assume pad is facing upwards

		Damn::RawNode BaseRaw, PressureRaw, SpecialRaw;

		if (!Damn::GetRawNodeFromScene(scene, "PadBase", BaseRaw, mResources->Ren3))
			throw PullException();

		if (!Damn::GetRawNodeFromScene(scene, "PadPressure", PressureRaw, mResources->Ren3))
			throw PullException();

		if (!Damn::GetRawNodeFromScene(scene, "PadSpecial", SpecialRaw, mResources->Ren3))
			throw PullException();

		m_BaseGeo = BaseRaw.Geometry;
		m_BaseMat = BaseRaw.Material;
		s_BaseGeo = m_BaseGeo;
		s_BaseMat = m_BaseMat;

		m_PressureGeo = PressureRaw.Geometry;
		m_PressureMat = PressureRaw.Material;
		s_PressureGeo = m_PressureGeo;
		s_PressureMat = m_PressureMat;

		m_SpecialGeo = SpecialRaw.Geometry;
		s_SpecialGeo = m_SpecialGeo;
	}
}

void Perviousity::Activators::PressurePad::EnsureShapes()
{
	if (!StaticShape)
		StaticShape = std::make_shared<btBoxShape>(btVector3(1.f, 0.05f, 1.f));

	if (!GhostShape)
		GhostShape = std::make_shared<btBoxShape>(btVector3(0.69f, 0.0625f, 0.69f));
}

Perviousity::Shapes::CubeyBoi::CubeyBoi(G1::IShapeThings tings, CubeyBoiThings things) : FullResourceHolder(tings.Resources), IShape(tings), m_CubeThings(things), m_ActiveTransform(things.Transform)
{
	if (!CubeShape)
		CubeShape = std::make_shared<btBoxShape>(btVector3(0.5f, 0.5f, 0.5f));

	if (m_CubeThings.Mass)
	{
		btVector3 localInertia;
		CubeShape->calculateLocalInertia(m_CubeThings.Mass, localInertia);

		m_CubeBody = std::make_shared<btRigidBody>(btRigidBody::btRigidBodyConstructionInfo(m_CubeThings.Mass, this, CubeShape.get(), localInertia));
		Container->RequestPhysicsCall(m_CubeBody, DYNAMIC_PERVIOUS | PAD_PRESSURER | INTERACTABLE, ENVIRONMENT | PLAYER | DYNAMIC_PERVIOUS | DYNAMIC_PERVIOUS_INVERTED | PERVIOUS_WALL | ACTIVATOR_PAD | INTERACTION_RAY);
	}
	else
	{
		m_CubeBody = std::make_shared<btRigidBody>(btRigidBody::btRigidBodyConstructionInfo(0.f, this, CubeShape.get()));
		Container->RequestPhysicsCall(m_CubeBody, ENVIRONMENT | PAD_PRESSURER, PLAYER | DYNAMIC_PERVIOUS | DYNAMIC_PERVIOUS_INVERTED | ACTIVATOR_PAD);
	}

	m_DrawCallKey = mResources->Ren3->AddDrawCall(Drawing::DrawCall{ DagNabbit::DefaultObjects::GetDefaultCubeGeo(mResources->Ren3), 0u, DagNabbit::DefaultObjects::GetDefaultMaterial(), &m_ActiveTransform, true, GetName() });

	m_CubeBody->setUserPointer(&m_Holder);
}

Perviousity::Shapes::CubeyBoi::~CubeyBoi()
{
	mResources->Ren3->RemoveDrawCall(m_DrawCallKey);
}

void Perviousity::Shapes::CubeyBoi::BeforeDraw()
{
}

void Perviousity::Shapes::CubeyBoi::AfterDraw()
{
	// Off the world check
	float out_of_bounds = DefaultOutOfBounds;
	if (auto p = std::any_cast<float>(mResources->Config->GetN(Config::Key{ "OutOfBoundsLevel" })))
		out_of_bounds = *p;

	if (GetPosition().y() < out_of_bounds)
	{
		// Reset Position, velocity and torque
		m_CubeBody->setLinearVelocity(btVector3{ 0.f, 0.f, 0.f });
		m_CubeBody->setAngularVelocity(btVector3{ 0.f, 0.f, 0.f });
		m_CubeBody->setWorldTransform(m_CubeThings.Transform);
		DINFO("Resetting cube");
		// Theoretically it should automatically update the m_ActiveTranform through the btMotionState overrides
	}
}

void Perviousity::Shapes::CubeyBoi::setWorldTransform(const btTransform & trans)
{
	trans.getOpenGLMatrix(m_ActiveTransform.ma);
}

void Perviousity::Shapes::CubeyBoi::getWorldTransform(btTransform & out) const
{
	out.setFromOpenGLMatrix(m_ActiveTransform.ma);
}

btVector3 Perviousity::Shapes::CubeyBoi::GetPosition() const
{
	return m_CubeBody->getWorldTransform().getOrigin();
}

btTransform Perviousity::Shapes::CubeyBoi::GetTransform() const
{
	return m_CubeBody->getWorldTransform();
}

void Perviousity::Shapes::CubeyBoi::SetPosition(btVector3 pos)
{
	m_CubeThings.Transform.dx = pos.x();
	m_CubeThings.Transform.dy = pos.y();
	m_CubeThings.Transform.dz = pos.z();
	btTransform updated = m_CubeBody->getWorldTransform();
	updated.setOrigin(pos);
	m_CubeBody->setWorldTransform(updated);
}

void Perviousity::Shapes::CubeyBoi::SetVelocity(btVector3 vel)
{
	m_CubeBody->setLinearVelocity(vel);
}

void Perviousity::Shapes::CubeyBoi::ApplyTorque(btVector3 torque)
{
	m_CubeBody->applyTorque(torque);
}

Pointer::f_ptr<Perviousity::Shapes::IPickupAble> Perviousity::Shapes::CubeyBoi::GetMe()
{
	if (Parent)
	{
		for (auto& child : Parent->children)
		{
			if (child.get() == this)
				return Pointer::f_ptr<Perviousity::Shapes::IPickupAble>(this, child.get_control());
		}
	}
	return nullptr;
}

btRigidBody* Perviousity::Shapes::CubeyBoi::GetBody()
{
	return m_CubeBody.get();
}

Perviousity::Activators::Ender::Ender(G1::IShapeThings ting) : IShape(ting), FullResourceHolder(ting.Resources)
{
}

void Perviousity::Activators::Ender::BeforeDraw()
{
}

void Perviousity::Activators::Ender::AfterDraw()
{
}

void Perviousity::Activators::Ender::Activate()
{
	mResources->Request->Request(Requests::Request{ "EndLevel" });
}

std::shared_ptr<Perviousity::Shapes::PlatformMaterial> Perviousity::Activators::GetActivatorMaterial()
{
	static std::list<std::shared_ptr<Perviousity::Shapes::PlatformMaterial>> Materials =
	{
		std::make_shared<Perviousity::Shapes::PlatformMaterial>(Material{floaty4{0.f, 0.f, 0.f, 0.f}, floaty4{0.f, 0.f, 0.f, 0.f}, floaty4{ 0.8f, 0.8f, 0.f, 1.f}, floaty4{0.2f, 0.2f, 0.2f, 15.f}, 1.f, 0}), // Nice Yellow
		std::make_shared<Perviousity::Shapes::PlatformMaterial>(Material{floaty4{0.f, 0.f, 0.f, 0.f}, floaty4{0.f, 0.f, 0.f, 0.f}, floaty4{ 0.7f, 0.2f, 0.2f, 1.f}, floaty4{ 0.2f, 0.2f, 0.2f, 15.f}, 1.f, 0}), // Good ol' red
		std::make_shared<Perviousity::Shapes::PlatformMaterial>(Material{floaty4{0.f, 0.f, 0.f, 0.f}, floaty4{0.f, 0.f, 0.f, 0.f}, floaty4{ 0.2f, 0.7f, 0.2f, 1.f}, floaty4{0.2f, 0.2f, 0.2f, 15.f}, 1.f, 0}), // Your average green
		std::make_shared<Perviousity::Shapes::PlatformMaterial>(Material{floaty4{0.f, 0.f, 0.f, 0.f}, floaty4{0.f, 0.f, 0.f, 0.f}, floaty4{0.2f, 0.2f, 0.7f, 1.f}, floaty4{ 0.2f, 0.2f, 0.2f, 15.f}, 1.f, 0}), // Blue
	};
	static std::shared_ptr<Perviousity::Shapes::PlatformMaterial> Backup = std::make_shared<Perviousity::Shapes::PlatformMaterial>(Material{ floaty4{ 0.f, 0.f, 0.f, 0.f }, floaty4{ 0.f, 0.f, 0.f, 0.f }, floaty4{ 0.f, 0.f, 0.f, 1.f }, floaty4{ 0.f, 0.f, 0.f, 1.f }, 1.f, 0 }); // Full Black
	for (auto &mat : Materials)
	{
		if (mat.use_count() == 1)
		{
			mat->Lost = false;
			return mat;
		}
	}
	
	Backup->Lost = false;
	return Backup;
}
