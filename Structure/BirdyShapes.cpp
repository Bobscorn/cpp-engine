#include "BirdyShapes.h"

#include "Helpers/MathHelper.h"

#include "Systems/Input/Input.h"

#include <random>

std::unique_ptr<btSphereShape> G1I::BirdyController::Shape = nullptr;

G1I::BirdyController::BirdyController(G1::IGSpace * container, CommonResources * resources) : IShape(container), FullResourceHolder(resources), PhysicsShape({ 0.f, 5.f, 0.f }), Cam(new Camera())
{
	Name = "Birdy Controller";
	Cam->SetLens(Math::QuatPiF, (float)*mResources->AspectRatio, IdealNearPlane, IdealFarPlane);
	Cam->SetPosLookUp({ 0.f, 1.f, 0.f }, { 0.f, 0.f, 1.f }, { 0.f, 1.f, 0.f });
	Container->SetCamera(Cam);

	if (!Shape)
		Shape = std::make_unique<btSphereShape>(btScalar(SphereRadius));

	btVector3 localInertia;
	Shape->calculateLocalInertia(btScalar(Mass), localInertia);

	RigidBody = std::make_shared<btRigidBody>(btRigidBody::btRigidBodyConstructionInfo(btScalar(Mass), this, Shape.get(), localInertia));
	//RigidBody->setAngularFactor(btScalar(0.0f));
	RigidBody->forceActivationState(DISABLE_DEACTIVATION);

	//Container->RequestPhysicsCall(Pointer::nef_ptr<btCollisionObject>(RigidBody), btBroadphaseProxy::DefaultFilter, -1);
	Container->RequestPhysicsCall(RigidBody, BIRDY_CONTROLLER_BITMASK, -1);

	mResources->Event->Add(this);
}

void G1I::BirdyController::BeforeDraw()
{
}

void G1I::BirdyController::AfterDraw()
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
			Cam->RotateY(float(float(mResources->MousePosition->x - *mResources->CenterX) * (*mResources->InverseWindowWidth)));
		}

		if (xchange || ychange)
		{
			SDL_WarpMouseInWindow(mResources->Window, *mResources->CenterX, *mResources->CenterY);
		}
	}
	if (Jump)
	{
		// Normal lame jump
		//RigidBody->applyCentralImpulse({ 0.f, JumpPower, 0.f });
		// Special cool jump
		btVector3 vel = RigidBody->getLinearVelocity();
		btVector3 NewVel = vel + JumpPower * btVector3{ 0.f, 1.f, 0.f } * RigidBody->getLinearFactor() * RigidBody->getInvMass();
		if (vel.y() < 0.f)
		{
			NewVel.setY(NewVel.getY() + RecoveryJumpPower * (vel.getY() * vel.getY()));
		}

		RigidBody->setLinearVelocity(NewVel);

		if (Sprint)
		{
			RigidBody->applyCentralForce({ 0.f, 0.f, -LungePower });
		}

		Jump = false;
	}
	if (Sprint)
		RigidBody->setLinearVelocity({ 0.f, fminf(RigidBody->getLinearVelocity().y(), MaxUpSpeed), -FastSpeed });
	else
		RigidBody->setLinearVelocity({ 0.f, fminf(RigidBody->getLinearVelocity().y(), MaxUpSpeed), -Speed });
}

bool G1I::BirdyController::Receive(Events::IEvent * event)
{
	return false;
}

bool G1I::BirdyController::Receive(Event::AfterPhysicsEvent * e)
{
	floaty3 ass(RigidBody->getWorldTransform().getOrigin());
	ass.z = -ass.z; // Does a lot
	Cam->SetPosition(ass);
	return true;
}

bool G1I::BirdyController::Receive(Event::KeyInput * e)
{
	if (e->State)
	{
		if (e->KeyCode == SDLK_SPACE)
			Jump = true;
		else if (e->KeyCode == SDLK_LSHIFT)
			Sprint = true;
	}
	else
	{
		if (e->KeyCode == SDLK_p)
		{
			Requests::Request ass("ToggleDebugDraw");
			Container->Request(ass);
		}
		else if (e->KeyCode == SDLK_o)
		{
			Requests::Request ass("ToggleRealDraw");
			Container->Request(ass);
		}
		else if (e->KeyCode == SDLK_r)
		{
			RigidBody->setWorldTransform(btTransform(btMatrix3x3::getIdentity(), { 0.f, 5.f, 0.f }));
		}
		else if (e->KeyCode == SDLK_LSHIFT)
			Sprint = false;
	}
	return true;
}

bool G1I::BirdyController::Receive(Event::MouseButton * e)
{
	if (e->Button == RMB)
	{
		mouse_locked = e->State;
	}
	return true;
}

bool G1I::BirdyController::Receive(Event::WindowFocusEvent * e)
{
	Sprint = false;
	mouse_locked = false;
	return true;
}

bool G1I::BirdyController::Receive(Event::ResizeEvent * e)
{
	(void)e;
	Cam->SetLens(Math::QuatPiF, float(*mResources->AspectRatio), IdealNearPlane, IdealFarPlane);
	return true;
}

G1I::BirdyObstacleInator::BirdyObstacleInator(G1::IGSpace * container, CommonResources * resources, Pointer::f_ptr<G1I::IBirdyObstaclePositionIndicator> posi) : FullResourceHolder(resources), IShape(container, "Birdy Obstacle Man"), PosInd(posi) 
{
	float z = -5.f;
	for (size_t i = 0; i < ObstacleCount; ++i)
	{
		Obstacles[i] = new BirdyObstacle(container, resources, z, GetRandomHeightGap(), GetRandomHeight());
		z -= GetRandomGap();
		this->AddChild(Obstacles[i]);
	}
}

void G1I::BirdyObstacleInator::AfterDraw()
{
	// Position Indicator is a nullable pointer, avoid null pointer exceptions
	if (!PosInd)
		return;

	size_t NewCount = 0;

	// Determine how many need to be updated

	for (size_t i = CurrentFront; i < ObstacleCount; ++i)
	{
		if ((Obstacles[i]->GetZ() - ObstacleLingerLength) > PosInd->GetZPos())
			++NewCount;
		else
			break;
	}

	// Don't do anything if there's nothing to be done
	if (!NewCount)
		return;

	UpdateObstacles(NewCount);
}

float G1I::BirdyObstacleInator::GetRandomHeight()
{
	int randi = std::rand();
	float randf = ((float)randi) / ((float)RAND_MAX);
	float randheightdif = randf * HeightDifference;
	return BaseHeight + randheightdif;
}

float G1I::BirdyObstacleInator::GetRandomGap()
{
	int randi = std::rand();
	float randf = ((float)randi) / ((float)RAND_MAX);
	float randgap = randf * PeriodDifference;
	return BasePeriod + randgap;
}

float G1I::BirdyObstacleInator::GetRandomHeightGap()
{
	constexpr float Difference = HeightGapUpper - HeightGapLower;

	int randi = std::rand();
	float randf = ((float)randi) / ((float)RAND_MAX);

	float heightgap = HeightGapLower + (randf * Difference); // fire in the disco
	return heightgap;
}

void G1I::BirdyObstacleInator::UpdateObstacles(size_t count)
{
	if (!count)
		return;

	if (count > ObstacleCount)
	{
		DINFO("Cannot update more obstacles than exist");
		count = ObstacleCount;
	}

	size_t NewFront = (CurrentFront + count) % ObstacleCount;
	size_t CurrentBack = (CurrentFront - 1);
	if (CurrentFront == 0)
		CurrentBack = ObstacleCount - 1;

	float z = Obstacles[CurrentBack]->GetZ() - GetRandomGap();

	if (Obstacles[CurrentBack]->GetZ() > PosInd->GetZPos())
		z = PosInd->GetZPos() - GetRandomGap();

	if (count == ObstacleCount)
	{
		for (size_t i = 0; i < ObstacleCount; ++i)
		{
			Obstacles[(i + CurrentFront) % ObstacleCount]->Update(z, GetRandomHeightGap(), GetRandomHeight());
			z -= GetRandomGap();
		}
	}
	else
	{
		for (size_t i = CurrentFront; i != NewFront; i = (i + 1) % ObstacleCount)
		{
			Obstacles[i]->Update(z, GetRandomHeightGap(), GetRandomHeight());
			z -= GetRandomGap();
		}
	}

	CurrentFront = NewFront;
}

void G1I::BirdyObstacle::Update(float zz, float heightgap, float y)
{
	this->z = zz;
	Lower->SetPosition({ 0.f, y, z });
	float yhigh = y + 4.f + heightgap;
	Upper->SetPosition({ 0.f, yhigh, z });
}

float G1I::BirdyObstacle::GetZ()
{
	return z;
}
