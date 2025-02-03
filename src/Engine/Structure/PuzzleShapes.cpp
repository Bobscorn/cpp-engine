#include "PuzzleShapes.h"

#include "Helpers/MathHelper.h"

#include "Drawing/GLRen.h"

#include "Systems/Input/Input.h"

#include <assert.h>
#include <algorithm>

std::unique_ptr<btCapsuleShape> Puzzle::G1I::PuzzlePlayer::Shape = nullptr;

Puzzle::G1I::PuzzlePlayer::PuzzlePlayer(G1::IGSpace * container, CommonResources * resources) : IShape(container), FullResourceHolder(resources), PhysicsShape({ 0.f, 1.9f, 0.f }), Cam(new Camera())
{
	JumpTimer.Reset();
	JumpTimer.Start();

	Name = "Puzzle Controller";
	Cam->SetLens(Math::QuatPiF, (float)*mResources->AspectRatio, IdealNearPlane, IdealFarPlane);
	Cam->SetPosLookUp({ 0.f, 1.f, 0.f }, { 0.f, 0.f, 1.f }, { 0.f, 1.f, 0.f });
	Container->SetCamera(Cam);

	if (!Shape)
		Shape = std::make_unique<btCapsuleShape>(btScalar(CapsuleRadius), btScalar(CapsuleHeight));

	btVector3 localInertia;
	Shape->calculateLocalInertia(btScalar(10.f), localInertia);

	RigidBody = std::make_shared<btRigidBody>(btRigidBody::btRigidBodyConstructionInfo(btScalar(10.f), this, Shape.get(), localInertia));
	RigidBody->setAngularFactor(btScalar(0.0f));
	RigidBody->forceActivationState(DISABLE_DEACTIVATION);

	Container->RequestPhysicsCall(RigidBody);

	mResources->Event->Add(this);
}

void Puzzle::G1I::PuzzlePlayer::BeforeDraw()
{
}

void Puzzle::G1I::PuzzlePlayer::AfterDraw()
{
	if (mouse_locked)
	{
		auto ychange = mResources->MousePosition->y != *mResources->CenterY, xchange = mResources->MousePosition->x != *mResources->CenterX;
		if (ychange)
		{
			Cam->Pitch(float(float(mResources->MousePosition->y - *mResources->CenterY) * (*mResources->InverseWindowHeight)));
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

	auto phys = Container->GetPhysicsWorld();

	if (auto dynamicsworld = phys.lock())
	{
		CheckGrounding(dynamicsworld.get());
		static bool prevground = false;
		if (groundy != prevground)
		{
			DINFO("RigidBody is now " + (groundy ? "on the ground" : "in the air"));
			prevground = groundy;
		}
		if (groundy)
		{
			JumpTimer.Tick();
			double jumpcooldown = JumpTimer.TotalTime();

			if (forwardness || rightness)
			{
				auto camforward = Cam->GetLook();
				auto camright = Cam->GetRight();
				btVector3 forwardv(camforward.x, 0.f, camforward.z);
				btVector3 rightv(camright.x, 0.f, camright.z);
				btVector3 target((forwardv * -forwardness) + (rightv * rightness));
				target.normalize();
				RigidBody->setLinearVelocity({ target.x() * walkpower, RigidBody->getLinearVelocity().y(), target.z() * walkpower });
				RigidBody->setDamping(0.f, RigidBody->getAngularDamping());
			}
			else if (jumpcooldown > JumpCooldown)
			{
				//if (RigidBody->getLinearDamping() != 0.9f)
				//	DINFO("Damping movement");
				RigidBody->setDamping(0.9f, RigidBody->getAngularDamping());
			}


			if (jump && (jumpcooldown > JumpCooldown || wasinair) && LastJustFrameID < (*mResources->UpdateID - 1) && (RigidBody->getLinearVelocity().getY() + 0.015f > 0.f))
			{
				wasinair = false;
				RigidBody->setDamping(0.f, RigidBody->getAngularDamping());
				RigidBody->applyCentralImpulse({ 0.f, JumpPower, 0.f });
				DINFO("Applied jump impulse, Rigid body is: " + (RigidBody->isActive() ? "Active" : "Inactive") + " and has velocity of: " + AsString(floaty3(RigidBody->getLinearVelocity())));
				JumpTimer.Reset();
				LastJustFrameID = *mResources->UpdateID;
			}
		}
		else
		{
			wasinair = true;
			RigidBody->setDamping(0.f, RigidBody->getAngularDamping());
			if (forwardness || rightness)
			{
				auto camforward = Cam->GetLook();
				auto camright = Cam->GetRight();
				btVector3 forwardv(camforward.x, 0.f, camforward.z);
				btVector3 rightv(camright.x, 0.f, camright.z);
				btVector3 target((forwardv * -forwardness) + (rightv * rightness));
				target.normalize();

				btVector3 vel = RigidBody->getLinearVelocity();
				vel.setY(0.f);
				if (!vel.fuzzyZero())
				{
					btVector3 nvel = vel.normalized();
					float mag = vel.length();
					float dot = target.dot(nvel);

					btVector3 inveldir = nvel * dot;

					if (mag > 1.f)
					{
						if (dot > 0.f) // Player trying to move with velocity
						{
							target = target - inveldir;
							target = target + (inveldir * (1.f / mag)); // Scale movement towards velocity down
						}
						else if (dot < 0.f) // Player trying to move opposite to velocity
						{
							target = target - inveldir;
							target = target + (inveldir * mag); // Scale movement away from velocity up
						}
						else // Player not trying to move towards to away from velocity
						{
							// do nothing
						}
					}
				}

				RigidBody->applyCentralForce(target * airpower);
			}
		}
	}
}

bool Puzzle::G1I::PuzzlePlayer::Receive(Events::IEvent * event)
{
	(void)event;
	return false;
}

bool Puzzle::G1I::PuzzlePlayer::Receive(Event::WindowFocusEvent * e)
{
	if (!e->GainedFocus && mouse_locked)
		UnlockMouse();
	rightness = forwardness = 0.f;
	forward = back = right = left = false;
	jump = false;

	return Events::RelevantEvent;
}

bool Puzzle::G1I::PuzzlePlayer::Receive(Event::AfterPhysicsEvent * e)
{
	(void)e;
	floaty3 ass(RigidBody->getWorldTransform().getOrigin());
	ass.y = ass.y + 0.25f * CapsuleHeight;
	ass.z = -ass.z; // Does a lot
	Cam->SetPosition(ass);

	static int CameraUpdateThing = 0;

	++CameraUpdateThing;

	if ((CameraUpdateThing % 20) == 0)
	{
		floaty3 Pos = Cam->GetPosition();
		//DINFO("Updating Camera with look: " + AsString(Cam->GetLook()) + " and ass: " + AsString(ass) + " with Camera Position: " + AsString(Cam->GetPosition()));
	}

	return Events::RelevantEvent;
}

bool Puzzle::G1I::PuzzlePlayer::Receive(Event::KeyInput * key)
{
	if (key->State)
	{
		// Key being pressed, check if key for opposite direction is being pressed and resolve that issue
		if (key->KeyCode == SDLK_w)
		{
			if (!back)
				forwardness = 1.f;
			else
				forwardness = 0.f;
			forward = true;
		}
		else if (key->KeyCode == SDLK_s)
		{
			if (!forward)
				forwardness = -1.f;
			else
				forwardness = 0.f;
			back = true;
		}
		else if (key->KeyCode == SDLK_a)
		{
			if (!right)
				rightness = -1.f;
			else
				rightness = 0.f;
			left = true;
		}
		else if (key->KeyCode == SDLK_d)
		{
			if (!left)
				rightness = 1.f;
			else
				rightness = 0.f;
			right = key->State;
		}
		else if (key->KeyCode == SDLK_SPACE)
		{
			jump = true;
		}
	}
	else
	{
		// Key being released, check if there was a conflict with the opposite key, and if so go direction of the opposite key (conflict meaning both were down)
		if (key->KeyCode == SDLK_w)
		{
			if (back)
				forwardness = -1.f;
			else
				forwardness = 0.f;
			forward = false;
		}
		else if (key->KeyCode == SDLK_s)
		{
			if (forward)
				forwardness = 1.f;
			else
				forwardness = 0.f;
			back = false;
		}
		else if (key->KeyCode == SDLK_a)
		{
			if (right)
				rightness = 1.f;
			else
				rightness = 0.f;
			left = false;
		}
		else if (key->KeyCode == SDLK_d)
		{
			if (left)
				rightness = -1.f;
			else
				rightness = 0.f;
			right = false;
		}
		else if (key->KeyCode == SDLK_SPACE)
		{
			jump = false;
		}
		else if (key->KeyCode == SDLK_p)
		{
			Requests::Request ass("ToggleDebugDraw");
			Container->Request(ass);
		}
		else if (key->KeyCode == SDLK_o)
		{
			Requests::Request ass("ToggleRealDraw");
			Container->Request(ass);
		}
		else if (key->KeyCode == SDLK_i)
		{
			outputcaminfo = !outputcaminfo;
		}
	}
	return Events::RelevantEvent;
}

bool Puzzle::G1I::PuzzlePlayer::Receive(Event::MouseButton * e)
{
	if (e->Button == RMB)
	{
		if (e->State && !mouse_locked)
			LockMouse();
		else if (mouse_locked)
			UnlockMouse();
	}
	return Events::RelevantEvent;
}

bool Puzzle::G1I::PuzzlePlayer::Receive(Event::ResizeEvent * e)
{
	(void)e;
	Cam->SetLens(Math::QuatPiF, float(*mResources->AspectRatio), IdealNearPlane, IdealFarPlane);
	return Events::RelevantEvent;
}

void Puzzle::G1I::PuzzlePlayer::LockMouse()
{
	if (mouse_locked)
		return;

	int x, y;
	SDL_GetMouseState(&x, &y);
	mouse_locked_location = floaty2::TLtoGL({ (float)x, (float)y }, { *mResources->HalfWindowWidth, *mResources->HalfWindowHeight });
	//DINFO("Locking mouse at location: (" + std::to_string(x) + ", " + std::to_string(y) + ") with GL Coords: " + AsString(mouse_locked_location) + " with window shit: (" + std::to_string(*mResources->HalfWindowWidth) + ", " + std::to_string(*mResources->HalfWindowHeight) + ")");
	SDL_WarpMouseInWindow(mResources->Window, *mResources->CenterX, *mResources->CenterY);
	SDL_ShowCursor(0);
	mouse_locked = true;
}

void Puzzle::G1I::PuzzlePlayer::UnlockMouse()
{
	if (!mouse_locked)
		return;

	SDL_ShowCursor(1);
	floaty2 TL = floaty2::GLtoTL(mouse_locked_location, *mResources->HalfWindowWidth, *mResources->HalfWindowHeight);
	//DINFO("Unlocking mouse at location: " + AsString(TL) + " with GL Coords: " + AsString(mouse_locked_location) + " with window shit: (" + std::to_string(*mResources->HalfWindowWidth) + ", " + std::to_string(*mResources->HalfWindowHeight) + ")");
	SDL_WarpMouseInWindow(mResources->Window, (int)std::roundf(TL.x), (int)std::roundf(TL.y));
	mouse_locked = false;
}


// based on http://www.bulletphysics.org/mediawiki-1.5.8/index.php?title=Collision_Callbacks_and_Triggers#contactTest
// Used to test if player character is on the ground
struct GroundCallback : public btCollisionWorld::ContactResultCallback
{
	GroundCallback(btRigidBody& tgtBody, btScalar mindot = 0.f) : btCollisionWorld::ContactResultCallback(), body(tgtBody), maxNormalDot(mindot) {}

	btRigidBody& body; // < The body the sensor is monitoring
	btScalar maxNormalDot;
	bool OnGround = false;
	btScalar foundDot = btScalar(0.0);
	btVector3 foundNormal = { 0,0,0 };

	virtual bool needsCollision(btBroadphaseProxy* proxy) const {
		// superclass will check m_collisionFilterGroup and m_collisionFilterMask
		if (!btCollisionWorld::ContactResultCallback::needsCollision(proxy))
			return false;
		// if passed filters, may also want to avoid contacts between constraints
		return body.checkCollideWithOverride(static_cast<btCollisionObject*>(proxy->m_clientObject));
	}

	//! Called with each contact for your own processing (e.g. test if contacts fall in within sensor parameters)
	virtual btScalar addSingleResult(btManifoldPoint& cp,
		const btCollisionObjectWrapper* colObj0, int partId0, int index0,
		const btCollisionObjectWrapper* colObj1, int partId1, int index1)
	{
		(void)partId0;
		(void)partId1;
		(void)index0;
		(void)index1;
		// Skip unnecessary dot products if its already proven to be on ground
		if (OnGround)
			return 0;

		if (colObj0->m_collisionObject == &body)
		{
			btScalar dot = cp.m_normalWorldOnB.dot({ 0.f, 1.f, 0.f });
			if (dot > maxNormalDot)
			{
				OnGround = true;
				foundDot = dot;
				foundNormal = cp.m_normalWorldOnB;
			}
		}
		else
		{
			assert(colObj1->m_collisionObject == &body && "body does not match either collision object");
		}

		// do stuff with the collision point
		return 0; // There was a planned purpose for the return value of addSingleResult, but it is not used so you can ignore it.
	}
};

bool Puzzle::G1I::PuzzlePlayer::CheckGrounding(btDynamicsWorld * world)
{
	GroundCallback callback(*RigidBody, btScalar(0.25f));
	world->contactTest(RigidBody.get(), callback);
	groundy = callback.OnGround;
	return groundy;
}

Puzzle::G1I::FirstRoom::FirstRoom(G1::IGSpace * container, CommonResources * resources, PuzzlePlayer * player, IStageManager *man) : IShape(container), FullResourceHolder(resources), Player(player), man(man),
	Floor(new ::G1I::CubeyShape(container, resources, FlooroofMaterial, { RoomRadius, HalfThickness, RoomRadius }, ::G1I::Kinematic, nullptr, FloorPosition)),
	Roof(new ::G1I::CubeyShape(container, resources, FlooroofMaterial, { RoomRadius, HalfThickness, RoomRadius }, ::G1I::Kinematic, nullptr, RoofPosition)),
	NorthWall(new ::G1I::CubeyShape(container, resources, WallMaterial, { RoomRadius, 0.5f * RoomHeight, HalfThickness }, ::G1I::Kinematic, nullptr, NorthWallPosition)),
	EastWall(new ::G1I::CubeyShape(container, resources, WallMaterial, { HalfThickness, 0.5f * RoomHeight, RoomRadius }, ::G1I::Kinematic, nullptr, EastWallPosition)),
	SouthWall(new ::G1I::CubeyShape(container, resources, WallMaterial, { RoomRadius, 0.5f * RoomHeight, HalfThickness }, ::G1I::Kinematic, nullptr, SouthWallPosition)),
	WestWall(new ::G1I::CubeyShape(container, resources, WallMaterial, { HalfThickness, 0.5f * RoomHeight, RoomRadius }, ::G1I::Kinematic, nullptr, WestWallPosition))
{
	float distance = Math::Map(Math::RandF(), 0.f, 1.f, MinPosDistance, MaxPosDistance);
	float angle = Math::Map(Math::RandF(), 0.f, 1.f, 0.f, Math::Pi2F);
	TargetPosition = { distance * sinf(angle), distance * cosf(angle) };
	TargetOrientation = floaty3::Normalized({ Math::RandF(), 0.f, Math::RandF() });
	this->AddChild(Floor);
	this->AddChild(Roof);
	this->AddChild(NorthWall);
	this->AddChild(EastWall);
	this->AddChild(SouthWall);
	this->AddChild(WestWall);
	ChangeTimer.Reset();
	DINFO("Target Position is: " + AsString(TargetPosition));
	DINFO("Target Orientation is: " + AsString(TargetOrientation));

	// Do Textures
	Textures tex;
	tex.DiffuseTexture.reset(new Drawing::SDLFileImage(resources, "walltex.jpg"));
	tex.BumpTexture.reset(new Drawing::SDLFileImage(resources, "walltex_DISP.png", 1));
	tex.SpecularPowerTexture.reset(new Drawing::SDLFileImage(resources, "walltex_SPEC.png", 1));
	tex.NormalTexture.reset(new Drawing::SDLFileImage(resources, "walltex_NRM.png", 3));

	size_t key = NorthWall->GetDrawCallKey();
	Drawing::DrawCall *dac = resources->Ren3->GetDrawCall(key);
	if (dac)
	{
		dac->textures = std::make_shared<Textures>(tex);
		dac->Material->TextureBitmap = BIT_DIFFUSE | BIT_BUMP | BIT_SPECULARPOWER | BIT_NORMAL;
	}
	key = EastWall->GetDrawCallKey();
	dac = resources->Ren3->GetDrawCall(key);
	if (dac)
	{
		dac->textures = std::make_shared<Textures>(tex);
		dac->Material->TextureBitmap = BIT_DIFFUSE | BIT_BUMP | BIT_SPECULARPOWER | BIT_NORMAL;
	}
	key = SouthWall->GetDrawCallKey();
	dac = resources->Ren3->GetDrawCall(key);
	if (dac)
	{
		dac->textures = std::make_shared<Textures>(tex);
		dac->Material->TextureBitmap = BIT_DIFFUSE | BIT_BUMP | BIT_SPECULARPOWER | BIT_NORMAL;
	}
	key = WestWall->GetDrawCallKey();
	dac = resources->Ren3->GetDrawCall(key);
	if (dac)
	{
		dac->textures = std::make_shared<Textures>(tex);
		dac->Material->TextureBitmap = BIT_DIFFUSE | BIT_BUMP | BIT_SPECULARPOWER | BIT_NORMAL;
	}

	tex.DiffuseTexture.reset(new Drawing::SDLFileImage(resources, "floortex.jpg"));
	tex.BumpTexture.reset(new Drawing::SDLFileImage(resources, "floortex_DISP.png", 1));
	tex.SpecularPowerTexture.reset(new Drawing::SDLFileImage(resources, "floortex_SPEC.png", 1));
	tex.NormalTexture.reset(new Drawing::SDLFileImage(resources, "floortex_NRM.png", 3));

	key = Floor->GetDrawCallKey();
	dac = resources->Ren3->GetDrawCall(key);
	if (dac)
	{
		dac->textures = std::make_shared<Textures>(tex);
		dac->Material->TextureBitmap = BIT_DIFFUSE | BIT_BUMP | BIT_SPECULARPOWER | BIT_NORMAL;
	}

	std::string puzzlestate;
	if (man && man->GetBonusKey("PuzzleState", puzzlestate))
	{
		if (puzzlestate == "FirstPuzzleEnd")
		{
			Complete();
		}
	}
}

void Puzzle::G1I::FirstRoom::AfterDraw()
{
	if (!Completed)
	{
		floaty3 Pos = Player->GetPosition();
		float PDelta = TargetPosition.DistanceFrom({ Pos.x, Pos.z });
		float xfac = Math::Map(PDelta, 0.f, MaxDistance, 0.f, MaxRotation);
		floaty3 Orientation = Player->GetOrientation();
		float ODelta = TargetOrientation.dot(floaty3::Normalized({ Orientation.x, 0.f, Orientation.z }));
		float yfac = Math::Map(ODelta, -1.f, 1.f, MaxRotation, 0.f);
#if false
		static float oldxfac = 0.f;
		static float oldyfax = 0.f;
		if (xfac != oldxfac)
		{
			DINFO("New XFac is: " + std::to_string(xfac) + " PDelta: " + std::to_string(PDelta) + " Player: " + AsString(Pos));
			oldxfac = xfac;
		}
		if (yfac != oldyfax)
		{
			DINFO("New YFac is: " + std::to_string(yfac) + " ODelta: " + std::to_string(ODelta) + " Player: " + AsString(Orientation));
			oldyfax = yfac;
		}
#endif
		Matrixy4x4 Transform = Matrixy4x4::Translate(-FloorPosition);
		Matrixy3x3 Rot = Matrixy3x3::RotationR({ xfac, 0.f, yfac });
		Transform = Matrixy4x4::MultiplyE(Transform, Rot);
		Transform = Matrixy4x4::MultiplyE(Transform, Matrixy4x4::Translate(FloorPosition));
		Matrixy4x4 RoomTrans = Rot;
		floaty3 RoofPos = Transform.Transform(RoofPosition);
		floaty3 NorthPos = Transform.Transform(NorthWallPosition);
		floaty3 EastPos = Transform.Transform(EastWallPosition);
		floaty3 SouthPos = Transform.Transform(SouthWallPosition);
		floaty3 WestPos = Transform.Transform(WestWallPosition);
		RoomTrans.dx = RoofPos.x; RoomTrans.dy = RoofPos.y; RoomTrans.dz = RoofPos.z;
		Roof->SetWorld(RoomTrans);
		RoomTrans.dx = NorthPos.x; RoomTrans.dy = NorthPos.y; RoomTrans.dz = NorthPos.z;
		NorthWall->SetWorld(RoomTrans);
		RoomTrans.dx = EastPos.x; RoomTrans.dy = EastPos.y; RoomTrans.dz = EastPos.z;
		EastWall->SetWorld(RoomTrans);
		RoomTrans.dx = SouthPos.x; RoomTrans.dy = SouthPos.y; RoomTrans.dz = SouthPos.z;
		SouthWall->SetWorld(RoomTrans);
		RoomTrans.dx = WestPos.x; RoomTrans.dy = WestPos.y; RoomTrans.dz = WestPos.z;
		WestWall->SetWorld(RoomTrans);
		RoomTrans.dx = FloorPosition.x; RoomTrans.dy = FloorPosition.y; RoomTrans.dz = FloorPosition.z;
		Floor->SetWorld(RoomTrans);
		//DINFO("Roof's angular velocity is: " + AsString(floaty3(Roof->GetBody()->getAngularVelocity())));

		if (XFacCloseEnough(xfac) && YFacCloseEnough(yfac))
			Win();
	}
	else
	{
		ChangeTimer.Tick();
		double time = ChangeTimer.TotalTime();
		
		float ypos = std::clamp(Math::Map((float)time, 0.f, TargetTime, 0.f, TargetY), TargetY, 0.f);

		if (Floor)
		{
			Floor->SetWorld(Matrixy4x4::Translate({ FloorPosition.x, FloorPosition.y + ypos, FloorPosition.z }));
			Roof->SetWorld(Matrixy4x4::Translate({ RoofPosition.x, RoofPosition.y + ypos, RoofPosition.z }));
		}

		if (ypos == TargetY)
			MoveOn();
	}
}

bool Puzzle::G1I::FirstRoom::XFacCloseEnough(float fac)
{
	// Epsilon of 20cm
	constexpr float Epsilon = Math::Map(0.2f, 0.f, MaxDistance, 0.f, MaxRotation);
	return fac - Epsilon < 0.f;
}

bool Puzzle::G1I::FirstRoom::YFacCloseEnough(float fac)
{
	// Epsilon of 3 degrees
	constexpr float Epsilon = 3.f * Math::DegToRadF;
	return fac - Epsilon < 0.f;
}

void Puzzle::G1I::FirstRoom::Win()
{
	DINFO("You won");
	Complete();
	
	if (man)
	{
		man->SetBonusData("PuzzleState", "FirstPuzzleEnd");
		man->Save();
	}
}

void Puzzle::G1I::FirstRoom::Complete()
{
	Roof->SetWorld(Matrixy4x4::Translate(RoofPosition));
	Floor->SetWorld(Matrixy4x4::Translate(FloorPosition));
	NorthWall->SetWorld(Matrixy4x4::Translate(NorthWallPosition));
	EastWall->SetWorld(Matrixy4x4::Translate(EastWallPosition));
	SouthWall->SetWorld(Matrixy4x4::Translate(SouthWallPosition));
	WestWall->SetWorld(Matrixy4x4::Translate(WestWallPosition));
	Completed = true;
	ChangeTimer.Reset();

	SecondFloor = new ::G1I::CubeyShape(Container, mResources, SecondRoom::FloorMaterial, SecondRoom::FloorExtents, ::G1I::Static, nullptr, { 0.f, TargetY, 0.f });
	this->AddChild(SecondFloor);
}

void Puzzle::G1I::FirstRoom::MoveOn()
{
	DINFO("Next puzzle it seems");
	this->RemoveChild(Floor);
	this->RemoveChild(Roof);
	Floor = Roof = nullptr;
	Finished = true;
}

Puzzle::G1I::FirstPuzzlePlace::FirstPuzzlePlace(G1::IGSpace * container, CommonResources * resources, IStageManager * man) : IShape(container), FullResourceHolder(resources), Manager(man), player(new PuzzlePlayer(container, resources))
{
	this->AddChild(player);
	std::string puzzle = "FirstPuzzle";
	man->GetBonusKey("PuzzleName", puzzle);

	if (puzzle == "SecondPuzzle")
	{
		second = new SecondRoom(container, resources, player, man);
		this->AddChild(second);
	}
	else
	{
		if (puzzle != "FirstPuzzle")
			DWARNING("Value stored in data's PuzzleName variable was not a valid puzzle, and was defaulted to FirstPuzzle");
		first = new FirstRoom(container, resources, player, man);
		this->AddChild(first);
	}
}

void Puzzle::G1I::FirstPuzzlePlace::AfterDraw()
{
	if (first)
	{
		if (first->ReadyForMore())
		{
			// Transition to second stage
			Manager->SetBonusData("PuzzleState", "SecondPuzzleStart");
			second = new SecondRoom(this->Container, mResources, player, Manager, first);
			this->RemoveChild(first);
			this->AddChild(second);
			first = nullptr;
		}
	}
}

Puzzle::G1I::SecondRoom::SecondRoom(G1::IGSpace * container, CommonResources * resources, PuzzlePlayer * player, IStageManager * man, FirstRoom * firstroom) : IShape(container, "Second Puzzle Room"), FullResourceHolder(resources), Player(player), Manager(man)
{
	if (firstroom)
	{
		size_t index = 0;
		for (size_t i = firstroom->children.size(); i-- > 0; )
			if (firstroom->children[i].get() == firstroom->SecondFloor)
			{
				index = i;
				break;
			}
		if (index != -1)
		{
			Floor = firstroom->SecondFloor;
			children.emplace_back(std::move(firstroom->children[index]));
		}
	}

	if (!Floor)
	{
		Floor = new ::G1I::CubeyShape(Container, mResources, FloorMaterial, FloorExtents, ::G1I::Static, nullptr, { 0.f, -20.f, 0.f });
		this->AddChild(Floor);
	}
}

void Puzzle::G1I::SecondRoom::AfterDraw()
{
}
