#include "TestShapes.h"

#include "Helpers/MathHelper.h"

#include "Systems/Input/Input.h"
#include "Systems/Requests/Requester.h"

#include <BulletCollision/CollisionDispatch/btGhostObject.h>

void Test::Controller::DoCam()
{
	if (m_mouse_locked)
	{
		auto ychange = mResources->MousePosition->y != *mResources->CenterY, xchange = mResources->MousePosition->x != *mResources->CenterX;
		if (ychange)
		{
			Cam->Pitch(-float(float(mResources->MousePosition->y - *mResources->CenterY) * (*mResources->InverseWindowHeight)));
		}

		if (xchange)
		{
			Cam->RotateY(-float(float(mResources->MousePosition->x - *mResources->CenterX) * (*mResources->InverseWindowWidth)));
		}

		if (xchange || ychange)
		{
			SDL_WarpMouseInWindow(mResources->Window, *mResources->CenterX, *mResources->CenterY);
		}
	}
}

void Test::Controller::UpdateCache()
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

Test::Controller::Controller(G1::IShapeThings things)
	: IShape(things)
	, FullResourceHolder(things.Resources)
	, btKinematicCharacterController(new btPairCachingGhostObject(), new btCapsuleShape(btScalar(0.3f), btScalar(1.5f)), 0.00001f, btVector3(0.f, 1.f, 0.f))
	, ControllerGhost(this->getGhostObject())
	, ControllerShape(this->m_convexShape)
	, Cam(new Camera())
{
	auto weak = Container->GetPhysicsWorld();

	auto world = weak.lock();
	if (world)
	{
		
		world->addAction(this);

		Cam->SetLens(Math::QuatPiF, (float)*mResources->AspectRatio, IdealNearPlane, IdealFarPlane);
		Cam->SetPosLookUp({ 0.f, 1.f, 0.f }, { 0.f, 0.f, 1.f }, { 0.f, 1.f, 0.f });
		Container->SetCamera(Cam);

		ControllerGhost->setCollisionShape(ControllerShape.get());
		ControllerGhost->setWorldTransform((Matrixy4x4)Matrixy3x3::RotationY(Math::HalfPiF));
		Container->RequestPhysicsCall(ControllerGhost, btCollisionObject::CO_GHOST_OBJECT, -1);
	}

	if (mResources->Event)
		mResources->Event->Add(this);
}

Test::Controller::~Controller()
{
}

bool Test::Controller::Receive(Events::IEvent * e)
{
	(void)e;
	return false;
}

bool Test::Controller::Receive(Event::KeyInput * key)
{
	if (key->KeyCode == SDLK_p)
	{
		if (key->State)
		{
			Requests::Request ass{ "ToggleDebugDraw" };
			mResources->Request->Request(ass);
		}
	}
	else if (m_CurrentState)
		return m_CurrentState->Receive(key);
	return true;
}

bool Test::Controller::Receive(Event::MouseButton * mb)
{
	if (mb->Button == RMB)
	{
		m_mouse_locked = mb->State;
	}
	return true;
}

bool Test::Controller::Receive(Event::AfterPhysicsEvent * post)
{
	(void)post;
	
	floaty3 ass(this->m_currentPosition);
	ass.y = ass.y + 0.25f * 1.5f;
	Cam->SetPosition(ass);
	return true;
}

bool Test::Controller::Receive(Event::WindowFocusEvent * win)
{
	if (!win->GainedFocus)
	{
		if (m_CurrentState)
			m_CurrentState->Reset();
		m_mouse_locked = false;
		//this->setVelocityForTimeInterval({ 0.f, 0.f, 0.f }, 0.f);
	}
	return true;
}

void Test::Controller::BeforeDraw()
{
	
}

void Test::Controller::AfterDraw()
{
	DoCam();

	UpdateCache();

	if (m_TargetState)
		m_CurrentState = std::move(m_TargetState);

	if (!m_CurrentState)
		m_CurrentState = std::make_unique<Test::WalkingState>(this);

	m_CurrentState->BeforePhysics((btScalar)*mResources->DeltaTime);
}

const Camera & Test::IControllerState::GetCamera() const
{
	return *m_control->Cam;
}

btVector3 Test::IControllerState::GetLook() const
{
	return m_control->m_Look;
}

btVector3 Test::IControllerState::GetHorizontalLook() const
{
	return m_control->m_LookHorizontal;
}

btVector3 Test::IControllerState::GetRight() const
{
	return m_control->m_Right;
}

btVector3 Test::IControllerState::GetHorizontalRight() const
{
	return m_control->m_RightHorizontal;
}

btVector3 Test::IControllerState::GetPosition() const
{
	return m_control->m_currentPosition;
}

G1::IGSpace * Test::IControllerState::GetGSpace() const
{
	return m_control->GetContainer();
}

btScalar Test::IControllerState::GetTargetUpdateInterval() const
{
	return btScalar(*m_control->mResources->TargetUpdateInterval);
}

bool Test::IControllerState::CanJump() const
{
	return m_control->canJump();
}

void Test::IControllerState::ChangeState(std::unique_ptr<IControllerState> newstate)
{
	m_control->m_TargetState = std::move(newstate);
}

void Test::IControllerState::SetWalkDirection(const btVector3 & dir)
{
	m_control->setWalkDirection(dir);
}

void Test::IControllerState::Jump(const btVector3 & impulse)
{
	m_control->jump(impulse);
}

void Test::IControllerState::DisableGravity()
{
	m_control->setUp({ 0.f, 0.f, 0.f });
}

void Test::IControllerState::EnableGravity()
{
	m_control->setUp({ 0.f, 1.f, 0.f });
}

bool Test::IControllerState::HasGravity()
{
	return m_control->getUp() == btVector3{ 0.f, 1.f, 0.f };
}

bool Test::WalkingState::Receive(Events::IEvent * e)
{
	(void)e;
	return false;
}

bool Test::WalkingState::Receive(Event::KeyInput * key)
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
			DINFO("Jumped");
			if (CanJump())
				Jump({ 0.f, 10.f, 0.f });
		}
	}
	else if (key->KeyCode == SDLK_LSHIFT)
	{
		m_sprint = key->State;
	}
	return false;
}

void Test::WalkingState::Reset()
{
	m_forwardness = m_rightness = 0.f;
	m_forward = m_back = m_right = m_left = m_sprint = false;
	SetWalkDirection({ 0.f, 0.f, 0.f });
}

void Test::WalkingState::BeforePhysics(btScalar dt)
{
	btVector3 walkdir = { 0.f, 0.f, 0.f };
	if (m_forwardness)
		walkdir += GetHorizontalLook() * m_forwardness;
	if (m_rightness)
		walkdir += GetHorizontalRight() * m_rightness;
	if (walkdir.length2() > SIMD_EPSILON * SIMD_EPSILON)
		walkdir.normalize();
	SetWalkDirection((walkdir * m_TotalWalkSpeed + walkdir * (btScalar)m_sprint * m_SprintMagnitude) * dt);
}

void Test::WalkingState::AfterPhysics()
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

bool Test::CeilingTest(Controller * control, Test::CeilTestStuff stuff)
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
		btVector3 from = ((btVector3)stuff.From) - btVector3{ 0.f, stuff.LowerAmount, 0.f};
		btVector3 to = from + btVector3{0.f, stuff.Height, 0.f};

		CeilingRayCallback callback{ from, to, control->getGhostObject() };

		world->rayTest(from, to, callback);

		return callback.HitCeiling();
	}
	return false;
}
