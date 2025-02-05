#include "VoxelPlayer.h"

#include "Structure/BulletBitmasks.h"

#include "Helpers/ProfileHelper.h"
#include "Helpers/BulletHelper.h"

#include "Systems/Events/Events.h"
#include "Systems/Input/Input.h"
#include "Config/Config.h"

#ifdef __linux__
#include <experimental/filesystem>
typedef std::experimental::filesystem::directory_iterator std_directory_iterator;
#else
#include <filesystem>
typedef std::filesystem::directory_iterator std_directory_iterator;
#endif

#ifdef CP_ENG_PLAYER_OUTPUT
#define PLAYER_INFO(x) DINFO("Player: " + x)
#else
#define PLAYER_INFO(x) ((int)0)
#endif

#include "VoxelInteractable.h"
#include "VoxelChunk.h"
#include "VoxelWorld.h"
#include "Drawing/VoxelStore.h"

std::unique_ptr<btCapsuleShape> Voxel::VoxelPlayer::SweepCapsule = nullptr;
std::unique_ptr<btCylinderShape> Voxel::VoxelPlayer::Capsule = nullptr;


Voxel::VoxelPlayer::VoxelPlayer(G1::IShapeThings things, Voxel::VoxelWorld *world, Voxel::VoxelPlayerStuff stuff)
	: IShape(things)
	, FullResourceHolder(things.Resources)
	, Entity(world)
	, Cam(new Camera())
	, m_FeetSource(Audio::CreateSauce())
	, m_FootstepSounds(GetFootstepSounds())
	, m_FootstepRandomizer(0ull, (m_FootstepSounds.empty() ? 0 : m_FootstepSounds.size() - 1))
	, m_Stuff(stuff)
{
	Name = "Puzzle Controller";
	Cam->SetLens(140.f * 0.5f * Math::DegToRadF, (float)*mResources->AspectRatio, IdealNearPlane, IdealFarPlane);
	Cam->SetPosLookUp(m_Stuff.InitialPosition, m_Stuff.InitialDirection, { 0.f, 1.f, 0.f });
	Container->SetCamera(Cam);

	if (!Capsule)
		Capsule = std::make_unique<btCylinderShape>(btVector3{ btScalar(CapsuleRadius), btScalar(CapsuleHeight), btScalar(CapsuleRadius) });

	if (!SweepCapsule)
		SweepCapsule = std::make_unique<btCapsuleShape>(btScalar(CapsuleRadius + CapsuleRadiusBonus), btScalar(RayFinalLength - CapsuleRadius - CapsuleRadiusBonus));

	btVector3 LocalInertia;
	Capsule->calculateLocalInertia(Fatness, LocalInertia);

	m_RigidBody = std::make_shared<btRigidBody>(btRigidBody::btRigidBodyConstructionInfo(Fatness, nullptr, Capsule.get(), LocalInertia));
	m_RigidBody->setAngularFactor(btScalar(0.0));
	m_RigidBody->setActivationState(DISABLE_DEACTIVATION);
	m_RigidBody->setFriction(0.f);
	m_RigidBody->setRestitution(0.f);
	m_RigidBody->setRollingFriction(0.f);
	m_RigidBody->setSpinningFriction(0.f);

	m_KinematicFeet = std::make_shared<btRigidBody>(btRigidBody::btRigidBodyConstructionInfo(0.f, &m_FeetState, SweepCapsule.get()));
	m_KinematicFeet->setCollisionFlags(m_KinematicFeet->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT);
	m_KinematicFeet->setActivationState(DISABLE_DEACTIVATION);

	Container->RequestPhysicsCall(m_RigidBody, PLAYER, ENVIRONMENT|ENTITY_GENERAL);
	Container->RequestPhysicsCall(m_KinematicFeet, PLAYER_FEET, PLAYER_FEET_DETECTOR);

	m_RigidBody->getWorldTransform().setOrigin(m_Stuff.InitialPosition);

	UpdateRayCache();

	m_PullTimer.Reset();
	m_PullTimer.Stop();

	mResources->Event->Add(this);
	mResources->Config->SafeSet({ "MouseSensitivityInverse" }, 1.f / 250.f);
}

Voxel::VoxelPlayer::~VoxelPlayer()
{
	UnlockMouse();
}

void Voxel::VoxelPlayer::BeforeDraw()
{
}

void Voxel::VoxelPlayer::AfterDraw()
{
	DoCam();

	TestForOutOfBounds();

	//DoPhys();

	DoSound();

	DoPickup();

	UpdateCache();

	if (!m_IsDashing)
		DoWalkFrame();
	else
		DoDashing();

	CheckForUnloadedChunk();
}

void Voxel::VoxelPlayer::TakeKnockbackDirect(floaty3 knock_back)
{
	m_RigidBody->applyCentralImpulse(knock_back);
}

bool Voxel::VoxelPlayer::HitByProjectile(Projectile *proj, floaty3 hit_point)
{
	(void)proj;
	(void)hit_point;
	return true;
}

floaty3 Voxel::VoxelPlayer::GetCentre()
{
	return floaty3(m_RigidBody->getWorldTransform().getOrigin());
}

void Voxel::VoxelPlayer::SetLookUp(floaty3 newLook, floaty3 newUp)
{
	Cam->SetPosLookUp(Cam->GetPosition(), newLook, newUp);
}

void Voxel::VoxelPlayer::SetPosition(floaty3 pos)
{
	Cam->SetPosition(pos);
	m_RigidBody->getWorldTransform().setOrigin(pos);
}

void Voxel::VoxelPlayer::SetVelocity(floaty3 newVel)
{
	m_RigidBody->setLinearVelocity(newVel);
}

bool Voxel::VoxelPlayer::Receive(Events::IEvent *event)
{
	(void)event;
	return false;
}

bool Voxel::VoxelPlayer::Receive(Event::WindowFocusEvent *e)
{
	if (!e->GainedFocus && m_MouseIsLocked)
		UnlockMouse();
	rightness = forwardness = 0.f;
	forward = back = right = left = false;

	ResetState();

	return Events::RelevantEvent;
}

bool Voxel::VoxelPlayer::Receive(Event::AfterPhysicsEvent *e)
{
	(void)e;
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

bool Voxel::VoxelPlayer::Receive(Event::KeyInput *key)
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
	else if (key->KeyCode == SDLK_SPACE)
	{
		if (key->State)
		{
			if (CanJump())
			{
				PLAYER_INFO("Jumped");
				Jump({ 0.f, 4.f, 0.f });
			}
		}
	}
	else if (key->KeyCode == SDLK_LSHIFT)
	{
		m_sprint = key->State;
	}
	else if (key->KeyCode == SDLK_x)
	{
		if (key->State)
		{
			if (CanDash())
			{
				PLAYER_INFO("Dashed");
				Dash(DashPower);
			}
		}
	}
	else if (key->KeyCode == SDLK_LCTRL)
	{
		SetCrouchState(key->State);
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
	else if (key->KeyCode == SDLK_1)
	{
		if (key->State)
		{
			m_SelectedBlockName = "wood";
		}
	}
	else if (key->KeyCode == SDLK_2)
	{
		if (key->State)
		{
			m_SelectedBlockName = "grass";
		}
	}
	else if (key->KeyCode == SDLK_3)
	{
		if (key->State)
		{
			m_SelectedBlockName = "lamp-base";
		}
	}
	else if (key->KeyCode == SDLK_4)
	{
		if (key->State)
		{
			m_SelectedBlockName = "lamp-pole";
		}
	}
	else if (key->KeyCode == SDLK_5)
	{
		if (key->State)
		{
			m_SelectedBlockName = "lamp-bend";
		}
	}

	return Events::RelevantEvent;
}

bool Voxel::VoxelPlayer::Receive(Event::MouseButton *e)
{
	if (e->Button == LMB && e->State)
	{
		if (m_MouseIsLocked)
		{
			TmpRayThing(true);
		}
		else
		{
			LockMouse();
		}
	}
	else if (e->Button == RMB && e->State)
	{
		if (m_MouseIsLocked)
		{
			TmpRayThing(false);
		}
		else
		{
			LockMouse();
		}
	}
	return Events::RelevantEvent;
}

void Voxel::VoxelPlayer::Superseded()
{
	if (m_MouseIsLocked)
	{
		UnlockMouse();
	}
}

bool Voxel::VoxelPlayer::Receive(Event::ResizeEvent *e)
{
	(void)e;
	Cam->SetLens(140.f * 0.5f * Math::DegToRadF, float(*mResources->AspectRatio), IdealNearPlane, IdealFarPlane);
	return Events::RelevantEvent;
}

void Voxel::VoxelPlayer::ShootTestRay(bool destroy)
{
	PLAYER_INFO("Player: Shooting Test Ray");
	btVector3 from = Cam->GetPosition();
	btVector3 to = from + (Cam->GetLook() * ShootDistance);
	btCollisionWorld::ClosestRayResultCallback rayboi(from, to);
	rayboi.m_collisionFilterMask = ENVIRONMENT;
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
				auto *point = reinterpret_cast<BulletHelp::NothingHolder *>(hitman->getUserPointer());
				if (point)
				{
					auto *thing = dynamic_cast<Voxel::ICube *>(point->Pointy);
					if (thing)
					{
						if (destroy)
						{
							auto coord = thing->GetWorldPos();
							thing->GetWorld()->SetCube(coord, VoxelStore::EmptyBlockData);
						}
						else
						{
							floaty3 pos{ rayboi.m_hitPointWorld + rayboi.m_hitNormalWorld * 0.25f };
							auto coord = thing->GetWorld()->GetBlockCoordFromPhys(pos);
							thing->GetWorld()->SetCube(coord, VoxelStore::Instance().GetDescOrEmpty("wood")->BlockData);
						}
					}
				}
			}
		}
		else
		{
			PLAYER_INFO("Player: Test Ray Didn't hit anything :(");
		}
	}
}

bool Voxel::VoxelPlayer::ShootInteractRay()
{
	/*btVector3 from = Cam->GetPosition();
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
			BulletHelp::NothingHolder *p = static_cast<BulletHelp::NothingHolder *>(rayboi.m_collisionObject->getUserPointer());
			if (p)
			{
				Activators::IActivatable *pp = dynamic_cast<Activators::IActivatable *>(p->Pointy);
				if (pp)
				{
					if (pp->AllowPlayerActivate())
						pp->Activate();

					return true;
				}
				else
				{
					Perviousity::Shapes::IPickupAble *pickup = dynamic_cast<Shapes::IPickupAble *>(p->Pointy);
					if (pickup)
					{
						for (auto &ontopof : m_PickupAblesOnTopOf)
							if (pickup == ontopof)
							{
								PLAYER_INFO("You're too fat to pick yourself up"); // Easter egg
								return true;
							}
						m_PickedUp = pickup->GetMe();
						m_PickedUp->GetBody()->forceActivationState(DISABLE_DEACTIVATION);

						return true;
					}
				}
			}
		}
	}*/

	return false;
}

void Voxel::VoxelPlayer::SpiritMode()
{
	PLAYER_INFO("Player: Spiriting");
	m_IsSpirit = true;
}

void Voxel::VoxelPlayer::NormalMode()
{
	PLAYER_INFO("Player: Despiriting");
	m_IsSpirit = false;
	m_RigidBody->setWorldTransform(btTransform(btQuaternion::getIdentity(), btVector3(Cam->GetPosition()) + btVector3(btScalar(0.0), btScalar(-0.45f * CapsuleHeight), btScalar(0.0))));
}

void Voxel::VoxelPlayer::Jump(const btVector3 &jump)
{
	DisableGroundPull();
	m_RigidBody->applyCentralImpulse(jump * Fatness);
}

void Voxel::VoxelPlayer::Dash(float power)
{
	if (m_DashesRemaining < 1)
		return;
	m_DashesRemaining--;
	m_DashTimer.Reset();
	m_DashTimer.Start();
	m_IsDashing = true;
	m_DashVector = m_LookHorizontal * power;
}

Voxel::VoxelPlayer::RayReturn Voxel::VoxelPlayer::FirstHit(float raylength)
{
	(void)raylength;
	RayReturn out;
	out.hold = nullptr;
	PLAYER_INFO("Player: Shooting ray");
	btVector3 from = Cam->GetPosition();
	btVector3 to = from + (Cam->GetLook() * ShootDistance);
	btCollisionWorld::ClosestRayResultCallback rayboi(from, to);
	rayboi.m_collisionFilterMask = ENVIRONMENT | ENTITY_GENERAL;
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
				auto *point = reinterpret_cast<BulletHelp::NothingHolder *>(hitman->getUserPointer());
				if (point)
				{
					out.hold = point;
					out.rayFrom = from;
					out.rayTo = to;
					out.hitPoint = rayboi.m_hitPointWorld;
					out.normal = rayboi.m_hitNormalWorld;
				}
			}
		}
	}
	return out;
}

void Voxel::VoxelPlayer::TmpRayThing(bool destroy)
{
	RayReturn ray = FirstHit(ShootDistance);

	if (ray.hold)
	{
		auto entity_ptr = dynamic_cast<Entity *>(ray.hold->Pointy);
		if (entity_ptr)
		{
			TmpEntityThing(ray, entity_ptr);
		}

		auto cube_ptr = dynamic_cast<VoxelChunk *>(ray.hold->Pointy);
		if (cube_ptr)
		{
			if (destroy)
				BreakBlock(ray);
			else
				PlaceBlock(ray);
		}
	}
	else
	{

	}
}

void Voxel::VoxelPlayer::TmpEntityThing(RayReturn ray, Entity *entity)
{
	//this->Attack(entity, 1.f);
	entity->TakeKnockback(floaty3{ ray.rayTo - ray.rayFrom }, this);
}

void Voxel::VoxelPlayer::BreakBlock(RayReturn ray)
{
	floaty3 pos{ ray.hitPoint - ray.normal * 0.25f };
	auto coord = m_World->GetBlockCoordFromPhys(pos);
	m_World->SetCube(coord, VoxelStore::EmptyBlockData);
}

void Voxel::VoxelPlayer::PlaceBlock(RayReturn ray)
{
	floaty3 pos{ ray.hitPoint + ray.normal * 0.25f };
	//auto thing = static_cast<Voxel::VoxelCube *>(ray.hold->Pointy);
	auto coord = m_World->GetBlockCoordFromPhys(pos);
	m_World->SetCube(coord, VoxelStore::Instance().GetDescOrEmpty(m_SelectedBlockName)->BlockData);
}

void Voxel::VoxelPlayer::RotateBlock(RayReturn ray)
{
	floaty3 pos{ ray.hitPoint - ray.normal * 0.25f };
	//auto thing = static_cast<Voxel::VoxelCube *>(ray.hold->Pointy);
	auto coord = m_World->GetBlockCoordFromPhys(pos);
	auto existing = m_World->GetCubeDataAt(coord);
	existing.Data.Rotation = quat4(btQuaternion(90.f * Math::DegToRadF, 0.f, 0.f)) * existing.Data.Rotation;
	if (existing.ID != 0)
	{
		m_World->SetCube(coord, existing);
	}
}

void Voxel::VoxelPlayer::SetCrouchState(bool state)
{
	m_Crouched = state;
	if (state)
		ApplyCrouchForce();
}

void Voxel::VoxelPlayer::DoWalkFrame()
{
	btVector3 walkdir = { 0.f, 0.f, 0.f };
	if (forwardness)
		walkdir += m_LookHorizontal * forwardness;
	if (rightness)
		walkdir += m_RightHorizontal * rightness;
	if (walkdir.length2() > SIMD_EPSILON *SIMD_EPSILON)
	{
		StampOnGround();
		walkdir.normalize();
	}
	btVector3 final_vel = (walkdir * m_Stuff.MovementSpeed + walkdir * (btScalar)m_sprint * m_SprintMagnitude);
	m_RigidBody->setLinearVelocity({ final_vel.x(), m_RigidBody->getLinearVelocity().y(), final_vel.z() });
}

void Voxel::VoxelPlayer::DoDashing()
{
	if (m_DashTimer.Stopped())
	{
		m_IsDashing = false;
		return;
	}

	m_DashTimer.Tick();
	if (m_DashTimer.TotalTime() > DashTime)
	{
		m_DashTimer.Stop();
		m_DashTimer.Reset();
		m_IsDashing = false;
	}
	m_RigidBody->setLinearVelocity(m_DashVector);
}

void Voxel::VoxelPlayer::ResetState()
{
	forwardness = rightness = 0.f;
	forward = back = right = left = m_sprint = false;

	m_Crouched = false;
	m_IsDashing = false;
	m_RigidBody->setLinearVelocity(btVector3{ 0.f, 0.f, 0.f });
}

bool Voxel::VoxelPlayer::CanJump()
{
	return m_OnGround;
}

bool Voxel::VoxelPlayer::CanDash()
{
	return m_DashesRemaining && !m_IsDashing;
}

void Voxel::VoxelPlayer::Die()
{
	PLAYER_INFO("Player: Placeholder death");
}

/*
void Voxel::VoxelPlayer::getWorldTransform(btTransform &t) const
{
	t = m_Trans;
}

void Voxel::VoxelPlayer::setWorldTransform(const btTransform &t)
{
	m_Trans = t;
}
*/
void Voxel::VoxelPlayer::DoCam()
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

void Voxel::VoxelPlayer::DoPhys()
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
				trans.setOrigin(trans.getOrigin() - btVector3{ 0.f, m_ConvexOffset + RayFinalLength * (m_Crouched ? CrouchScale : 1.f), 0.f });
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

void Voxel::VoxelPlayer::DoPickup()
{
	/*if (m_PickedUp)
	{
		for (auto &ontopof : m_PickupAblesOnTopOf)
			if (m_PickedUp.get() == ontopof)
			{
				PLAYER_INFO("Oh very clever, pick it up, and *then* hop on it, too bad"); // Easter egg
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
		*/
		/*btVector3 angvel = { Math::DifAngles(camx, px) * s_PickupSpinFactor, Math::DifAngles(camy, py) * s_PickupSpinFactor, Math::DifAngles(camz, pz) * s_PickupSpinFactor };
		float strength = angvel.length2();
		static float last_strength = 100.f;
		static float last_camx = 0.f, last_camy = 0.f, last_camz = 0.f, last_px = 0.f, last_py = 0.f, last_pz = 0.f;
		if (strength - 0.2f > last_strength)
			PLAYER_INFO("Got em with current: (" + std::to_string(last_camx) + ", " + std::to_string(last_camy) + ", " + std::to_string(last_camz) + "), (" + std::to_string(last_px) + ", " + std::to_string(last_py) + ", " + std::to_string(last_pz) + ")");
		PLAYER_INFO("Angular Velocity Strength: " + std::to_string(strength));
		last_strength = strength;
		last_camx = camx; last_camy = camy; last_camz = camz;
		last_px = px; last_py = py; last_pz = pz;*/
	/*
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
		xpos = trans.getBasis() * btVector3 { 1.f, 0.f, 0.f };
		xneg = xpos * -1.f;
		ypos = trans.getBasis() * btVector3 { 0.f, 1.f, 0.f };
		yneg = ypos * -1.f;
		zpos = trans.getBasis() * btVector3 { 0.f, 0.f, 1.f };
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
	}*/
}

void Voxel::VoxelPlayer::UpdateCache()
{
	auto look = Cam->GetLook();
	m_Look = look;
	m_LookHorizontal.setX(look.x);
	m_LookHorizontal.setZ(look.z);
	if (m_Look.getY() < (1.f - SIMD_EPSILON))
		m_LookHorizontal.normalize();

	auto rightDir = Cam->GetRight();
	m_Right = rightDir;
	m_RightHorizontal.setX(rightDir.x);
	m_RightHorizontal.setZ(rightDir.z);
	if (m_Right.getY() < (1.0 - SIMD_EPSILON))
		m_RightHorizontal.normalize();
}

void Voxel::VoxelPlayer::DoSound()
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

void Voxel::VoxelPlayer::TestForOutOfBounds()
{
	// Test for out of bounds

	float out_of_bounds = -200.f;
	if (auto plevel = std::any_cast<float>(mResources->Config->GetN(Config::Key{ "OutOfBoundsLevel" })))
		out_of_bounds = *plevel;

	if (Cam->GetPosition().y < out_of_bounds)
	{
		Cam->SetPosLookUp(m_Stuff.InitialPosition, m_Stuff.InitialDirection, { 0.f, 1.f, 0.f });
		m_RigidBody->getWorldTransform().setOrigin(m_Stuff.InitialPosition);
		m_RigidBody->setLinearVelocity({ 0.f, 0.f, 0.f });
	}
}

void Voxel::VoxelPlayer::CheckForUnloadedChunk()
{
	auto pos = m_World->GetBlockCoordFromPhys(GetPosition());
	auto chunkState = m_World->GetChunkStatus(pos.Chunk);
	if (chunkState == VoxelWorld::NOT_IN_WORLD)
	{
		//m_World->LoadChunk(pos.Chunk);
		(void)0;
	}
	else if (chunkState == VoxelWorld::IN_WORLD_NOT_LOADED)
	{
		m_RigidBody->setLinearVelocity(btVector3(0.f, 0.f, 0.f));
	}
}

void Voxel::VoxelPlayer::ResetPickup()
{
	/*if (m_PickedUp)
	{
		m_PickedUp->GetBody()->activate(true);
		m_PickedUp.reset();
	}*/
}

struct GroundNotMeCallback : btCollisionWorld::ClosestRayResultCallback
{
protected:
	btRigidBody *m_me = nullptr;
	btScalar m_minDot = btScalar(0.0);
	btVector3 m_up;
	//std::vector<Perviousity::Shapes::IPickupAble *> &Pickupables;
public:
	GroundNotMeCallback(btRigidBody *me, /*std::vector<Perviousity::Shapes::IPickupAble *> &pickupables, */btScalar minDot, const btVector3 &up, const btVector3 &from, const btVector3 &to) : ClosestRayResultCallback(from, to), m_me(me), m_minDot(minDot), m_up(up) {}//, Pickupables(pickupables) {}


	virtual btScalar addSingleResult(btCollisionWorld::LocalRayResult &rayresult, bool normalInWorld) override
	{
		if (rayresult.m_collisionObject == m_me)
		{
			return btScalar(1.0);
		}

		BulletHelp::NothingHolder *p = static_cast<BulletHelp::NothingHolder *>(rayresult.m_collisionObject->getUserPointer());
		if (p)
		{
			//auto *pp = dynamic_cast<Perviousity::Shapes::IPickupAble *>(p->Pointy);
			//if (pp)
			//{
				//Pickupables.emplace_back(pp);
			//}
		}

		if (normalInWorld)
		{
			if (rayresult.m_hitNormalLocal.dot(m_up) < m_minDot)
				return btScalar(1.0);
		}
		else
			if (m_up.dot(rayresult.m_collisionObject->getWorldTransform().getBasis() * rayresult.m_hitNormalLocal) < m_minDot)
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
	//std::vector<Perviousity::Shapes::IPickupAble *> &Pickupables;
public:
	GroundNotMeConvexCallbackRecordPickupables(btRigidBody *me, /*std::vector<Perviousity::Shapes::IPickupAble *> &pickupables, */btScalar minDot, const btVector3 &up, const btVector3 &from, const btVector3 &to) : ClosestConvexResultCallback(from, to), m_me(me), m_minDot(minDot), m_up(up) {}//, Pickupables(pickupables) {}


	virtual btScalar addSingleResult(btCollisionWorld::LocalConvexResult &convexresult, bool normalInWorld) override
	{
		if (convexresult.m_hitCollisionObject == m_me)
		{
			return btScalar(1.0);
		}

		BulletHelp::NothingHolder *p = static_cast<BulletHelp::NothingHolder *>(convexresult.m_hitCollisionObject->getUserPointer());
		if (p)
		{
			//auto *pp = dynamic_cast<Perviousity::Shapes::IPickupAble *>(p->Pointy);
			//if (pp)
			//{
			//	Pickupables.emplace_back(pp);
			//}
		}

		if (normalInWorld)
		{
			if (convexresult.m_hitNormalLocal.dot(m_up) < m_minDot)
				return btScalar(1.0);
		}
		else
			if (m_up.dot(convexresult.m_hitCollisionObject->getWorldTransform().getBasis() * convexresult.m_hitNormalLocal) < m_minDot)
				return btScalar(1.0);

		return ClosestConvexResultCallback::addSingleResult(convexresult, normalInWorld);
	}

};

void Voxel::VoxelPlayer::UpdateRayCache()
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

void Voxel::VoxelPlayer::DoFeetRays()
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

	PROFILE_PUSH("Calculating Determinant");
	float determinant = GetDeterminant(world.get(), &m_DeepestNormal);
	PROFILE_POP();

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
				//	PLAYER_INFO("Applied Slow Spring Force (GroundPull disabled)");
				//	RigidBody->applyCentralForce(GetRotatedForce({ 0.f, SpringKSlowUp * x, 0.f }));
				//}
				//else
				//{
					// Spring math
				float springfac = GetSpringFactor(vel.getY());
				PLAYER_INFO("Applied Spring Force (fac: " + std::to_string(springfac) + ", GroundPull disabled)");
				m_RigidBody->applyCentralForce(GetRotatedForce({ 0.f, SpringK * x * springfac, 0.f }));
				//}
			}
			else
			{
				PLAYER_INFO("Determinant Beyond Spring Zone, GroundPull is disabled, and y velocity is upwards");
				m_OnGround = false;
			}
		}
		else
		{
			const auto &C = determinant;
			if (C > FeetRayStart &&C < FeetRayEnd)
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
					PLAYER_INFO("Pulled down Player by: " + std::to_string(movedown) + ", clipped");
				}
				else
					PLAYER_INFO("Pulled down the Player by: " + std::to_string(movedown) + ", unclipped");
				trans.setOrigin(trans.getOrigin() + btVector3{ 0.f, -(movedown + 0.25f * DeadZoneLength), 0.f });
				m_RigidBody->setWorldTransform(trans);
				m_RigidBody->setLinearVelocity({ vel.x(), 0.f, vel.z() });
				//}
				//else
				//{
				//	PLAYER_INFO("Did nothing as pulldown amount was too small (<" + std::to_string(MinPullDown));
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
					PLAYER_INFO("Did bounce fix");
				}
				else
				{
					//btVector3 vel = RigidBody->getLinearVelocity();
					//if (vel.getY() > 0.f)
					//{
					//	PLAYER_INFO("Applied Slow Spring Force");
					//	RigidBody->applyCentralForce(GetRotatedForce({ 0.f, SpringKSlowUp * x, 0.f }));
					//}
					//else
					//{
					//	// Spring math
					//	PLAYER_INFO("Applied Normal Spring Force");
					//	RigidBody->applyCentralForce(GetRotatedForce({ 0.f, SpringK * x, 0.f }));
					//}
					float springfac = GetSpringFactor(m_RigidBody->getLinearVelocity().getY());
					PLAYER_INFO("Applied Spring Force (fac: " + std::to_string(springfac) + ")");
					m_RigidBody->applyCentralForce(GetRotatedForce({ 0.f, SpringK * x * springfac, 0.f }));
				}
			}
			else
			{
				static size_t LastDeadZoneUpdate = 0ull;
				static float DeadZonedDeterminant = 1.1f;
				if (LastDeadZoneUpdate != (*mResources->UpdateID - 1) || (determinant - 0.005f) > DeadZonedDeterminant || (determinant + 0.005f) < DeadZonedDeterminant)
				{
					PLAYER_INFO("Dead Zoned with " + std::to_string(determinant));
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
		m_DashesRemaining = 1;
		if (m_RigidBody->getGravity().length2() > (SIMD_EPSILON * SIMD_EPSILON))
		{
			PLAYER_INFO("Disabling Gravity");
			m_RigidBody->setGravity({ 0.f, 0.f, 0.f });
			m_RigidBody->setFlags(m_RigidBody->getFlags() & (~BT_DISABLE_WORLD_GRAVITY));
		}
	}
	else
	{
		if (m_RigidBody->getGravity() != btVector3{ 0.f, -9.8f, 0.f })
		{
			PLAYER_INFO("Re-enabling Gravity");
			m_RigidBody->setGravity({ 0.f, -9.8f, 0.f });
			m_RigidBody->setFlags(m_RigidBody->getFlags() | BT_DISABLE_WORLD_GRAVITY);
		}
	}
	PROFILE_POP();
}

btVector3 Voxel::VoxelPlayer::GetRotatedForce(btVector3 in)
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
		PLAYER_INFO("Input rotation: " + AsString(floaty3{ in }) + ", Norm: " + AsString(floaty3{ m_DeepestNormal }) + ", Rotated: " + AsString(floaty3{ out }));
	return out;
}

void Voxel::VoxelPlayer::DisableGroundPull()
{
	PLAYER_INFO("Disabling Ground Pull");
	m_DisableGroundPull = true;
	m_PullTimer.Reset();
	m_PullTimer.Start();
}

void Voxel::VoxelPlayer::EnableGroundPull()
{
	m_PullTimer.Tick();
	if (m_PullTimer.TotalTime() > MinPullStart)
	{
		PLAYER_INFO("Enabling Ground pull");
		m_DisableGroundPull = false;
	}
	else
	{
		PLAYER_INFO("Too early to enable ground pull");
	}
}

void Voxel::VoxelPlayer::ApplyCrouchForce()
{
	m_RigidBody->applyCentralForce(btVector3{ 0.f, -CrouchForce * Fatness, 0.f });
}

void Voxel::VoxelPlayer::LockMouse()
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

void Voxel::VoxelPlayer::UnlockMouse()
{
	if (!m_MouseIsLocked)
		return;

	SDL_ShowCursor(1);
	floaty2 TL = floaty2::GLtoTL(mouse_locked_location, *mResources->HalfWindowWidth, *mResources->HalfWindowHeight);
	SDL_WarpMouseInWindow(mResources->Window, (int)std::roundf(TL.x), (int)std::roundf(TL.y));
	m_MouseIsLocked = false;
}

void Voxel::VoxelPlayer::StampOnGround()
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

float Voxel::VoxelPlayer::GetDeterminant(btCollisionWorld *world, btVector3 *deepestnormal)
{
	if (m_UseRayTest)
	{
		float running_sum = 0.f;
		size_t counted_rays = 0u;
		float closest = 1.f;

		for (size_t i = 0; i < Raycount; ++i)
		{
			btVector3 from = m_RigidBody->getWorldTransform().getOrigin() + m_RayOffset[i];
			btVector3 to = from + btVector3{ 0.f, -RayFinalLength * m_Crouched ? CrouchScale : 1.f, 0.f };
			//m_PickupAblesOnTopOf.clear();
			GroundNotMeCallback callback{ m_RigidBody.get(), /*m_PickupAblesOnTopOf, */MinGroundDot, {0.f, 1.f, 0.f}, from, to };
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
		//m_PickupAblesOnTopOf.clear();
		GroundNotMeConvexCallbackRecordPickupables callback{ m_RigidBody.get(), /*m_PickupAblesOnTopOf, */MinGroundDot, {0.f, 1.f, 0.f}, fromv, tov };
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
}

float Voxel::VoxelPlayer::GetSpringFactor(btScalar YVel)
{
	return fmaxf(0.01f, fminf(1.f, Math::Map(YVel, -2.f, 2.f, 1.f, 0.f)));
}

float Voxel::VoxelPlayer::GetNewYVelSpring(btScalar current, btScalar time)
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

float Voxel::VoxelPlayer::GetNewYStage1(btScalar current, btScalar time, btScalar &out)
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

float Voxel::VoxelPlayer::GetNewYStage2(btScalar current, btScalar time, btScalar &out)
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

float Voxel::VoxelPlayer::GetNewYStage3(btScalar current, btScalar time, btScalar &out)
{
	//constexpr static float Stage4Start = SOMEBULLSHIT;

	out += log10f(current) * time;
	return 0.f;
}

std::vector<Audio::ALBufferI> Voxel::VoxelPlayer::GetFootstepSounds()
{
	std::vector<Audio::ALBufferI> sounds;
	try
	{
		std::error_code error;
		std_directory_iterator footsteps{ mResources->WorkingDirectory + "Sounds/Footsteps", error };
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
		PLAYER_INFO("Loaded " + std::to_string(count) + " footstep sounds");
	}
	catch (const std::bad_alloc & e)
	{
		(void)e;
		DWARNING("Could not map Sounds/Footsteps/ directory");
	}
	return sounds;
}