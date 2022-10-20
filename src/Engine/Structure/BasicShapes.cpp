//#include "Thing.h"
#include "BasicShapes.h"

#include "Helpers/VectorHelper.h"
#include "Helpers/MathHelper.h"
#include "Helpers/StringHelper.h"
#include "Helpers/ProfileHelper.h"

#include "Systems/Events/PhysicsEvent.h"
#include "Systems/Input/Input.h"
#include "Systems/Requests/Requester.h"

#include "Drawing/GLRen.h"
#include "Drawing/Material.h"

#if false
#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/Logger.hpp>
#include <assimp/DefaultLogger.hpp>
#include <assimp/postprocess.h>
#endif

#include <assert.h>

#ifdef __linux__
#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_mouse.h>
#else
#include <SDL_keycode.h>
#include <SDL_mouse.h>
#endif

std::unique_ptr<btCollisionShape> G1I::CheapCubeShape::bShape = nullptr;
std::unique_ptr<btCollisionShape> G1I::EnviroShape::Shape = nullptr;
std::unique_ptr<btCapsuleShape> G1I::DynamicCharacterController::Shape = nullptr;

std::array<bool, LIGHT_COUNT> G1I::LightShape::TakenLights = { 0 };

G1I::RenderShape::RenderShape(G1::IGSpace *container, std::vector<FullVertex> vertices, std::vector<unsigned int> indices, Material mat, Matrixy4x4 trans) : World(trans)
{
	(void)container;
	Drawing::DrawCall call = mResources->Ren3->CreateDrawCall(vertices, indices, mat, &World);
	call.DebugString = GetName();
	DrawCallKey = mResources->Ren3->AddDrawCall(call);
}

G1I::RenderShape::RenderShape(G1::IGSpace * container, std::vector<FullVertex> vertices, std::vector<unsigned int> indices, std::shared_ptr<Material> mat, Matrixy4x4 trans) : World(trans)
{
	(void)container;
	Drawing::DrawCall ass = mResources->Ren3->CreateDrawCall(vertices, indices, &World);
	ass.Material = mat;
	ass.DebugString = GetName();
	DrawCallKey = mResources->Ren3->AddDrawCall(ass);
}

G1I::RenderShape::RenderShape(G1::IGSpace * container, Drawing::DrawCall call)
{
	(void)container;
	if (!call.Matrix)
		call.Matrix = &World;

	if (call.DebugString.empty())
		call.DebugString = GetName();
	DrawCallKey = mResources->Ren3->AddDrawCall(call);
}

G1I::RenderShape::RenderShape(G1::IGSpace * container, std::string filename)
{
	(void)container;
	(void)filename;
	// deprecated code
	abort();
#if false
#ifdef _DEBUG
	Assimp::DefaultLogger::create("AssimpLog.txt", Assimp::Logger::LogSeverity::VERBOSE);
#endif

	Assimp::Importer ImportGuy;

	const aiScene *Scene = ImportGuy.ReadFile(filename, aiProcess_ValidateDataStructure |
		aiProcess_GenSmoothNormals |
		aiProcess_CalcTangentSpace |
		aiProcess_Triangulate |
		aiProcess_JoinIdenticalVertices |
		aiProcess_SortByPType);

#ifdef _DEBUG
	Assimp::DefaultLogger::kill();
#endif

	bool QuickMaths = (bool)42;
#endif
}

void G1I::RenderShape::SetWorld(Matrixy4x4 trans)
{
	World = trans;
}

Matrixy4x4 G1I::RenderShape::GetWorld()
{
	return World;
}

size_t G1I::RenderShape::GetDrawCallKey()
{
	return DrawCallKey;
}

void G1I::PhysicsRenderShape::getWorldTransform(btTransform & t) const
{
	t.setFromOpenGLMatrix(World.ma);
}

void G1I::PhysicsRenderShape::setWorldTransform(const btTransform & t)
{
	t.getOpenGLMatrix(World.ma);
}

floaty3 G1I::PhysicsRenderShape::GetPosition()
{
	return { World.dx, World.dy, World.dz };
}

G1I::CheapCubeShape::CheapCubeShape(G1::IGSpace *container, CommonResources *resources, Material mat, Matrixy4x4 trans, bool dynamic) : IShape(container), FullResourceHolder(resources), PhysicsRenderShape(container, std::vector<FullVertex>(CubeVertices.cbegin(), CubeVertices.cend()), std::vector<unsigned int>(CubeIndices.cbegin(), CubeIndices.cend()), mat, trans), Dynamic(dynamic)
{
	if (!bShape)
		bShape = std::make_unique<btBoxShape>(btVector3(0.5f, 0.5f, 0.5f));

	if (Dynamic)
	{
		btVector3 localInertia;
		bShape->calculateLocalInertia(btScalar(1.f), localInertia);

		RigidBody = std::make_shared<btRigidBody>(btRigidBody::btRigidBodyConstructionInfo(btScalar(1.f), this, bShape.get(), localInertia));
	}
	else
	{
		RigidBody = std::make_shared<btRigidBody>(btRigidBody::btRigidBodyConstructionInfo(btScalar(0.f), this, bShape.get()));
	}

	Container->RequestPhysicsCall(RigidBody);
}

G1I::NormalCamera::NormalCamera(G1::IGSpace *container, CommonResources *resources) : FullResourceHolder(resources), IShape(container, "NormalCamera"), IEventListener({ Events::MouseMoveEvent, Events::MouseButtonEvent, Events::MouseScrollEvent, Events::WindowFocusEvent }), Cam(new Camera())
{
	Container->SetCamera(Cam);
	Cam->SetLens(Math::HalfPiF, (float)*mResources->AspectRatio, IdealNearPlane, IdealFarPlane);
	Cam->SetPosLookUp({ 0.f, 0.f, -10.f }, { 0.f, 0.f, 1.f }, { 0.f, 1.f, 0.f });

	mResources->Event->Add(this);
}

void G1I::NormalCamera::BeforeDraw()
{
	if (dy)
	{
		Cam->Pitch(float(dy * multiplier));
		dy = 0.0;
	}

	if (dx)
	{
		Cam->RotateY(float(dx * multiplier));
		dx = 0.0;
	}

	if (mouse_locked)
	{
		SDL_WarpMouseInWindow(mResources->Window, *mResources->CenterX, *mResources->CenterY);
	}
}

void G1I::NormalCamera::AfterDraw()
{
}

bool G1I::NormalCamera::Receive(Events::IEvent * event)
{
	if (auto *move = Events::ConvertEvent<Event::MouseMove>(event))
	{
		if (auto *button = Events::ConvertEvent<Event::MouseButton>(event))
		{
			if (button->Button == LMB)
			{
				mouse_locked = button->State;
			}
		}
		else if (auto *scroll = Events::ConvertEvent<Event::MouseWheel>(event))
		{
			multiplier += float(0.1f * scroll->Scrollage);
			multiplier = fmaxf(0.1f, fminf(5.f, multiplier));
		}
		else if (mouse_locked)
		{
			float tmpx = float(move->_X - *mResources->CenterX);
			if (abs(tmpx) > 1)
				dx += tmpx;

			float tmpy = float(move->_Y - *mResources->CenterY);
			if (abs(tmpy) > 1)
			dy += tmpy;
		}
		return Events::RelevantEvent;
	}	
	else if (auto *win_focus = Events::ConvertEvent<Event::WindowFocusEvent>(event))
	{
		mouse_locked &= win_focus->GainedFocus;

		return Events::RelevantEvent;
	}
	return Events::IrrelevantEvent;
}

G1I::DynamicCharacterController::DynamicCharacterController(G1::IGSpace *container, CommonResources *resources) : IShape(container), FullResourceHolder(resources), PhysicsShape({ 0.f, 20.f, 0.f }), Cam(new Camera())
{
	JumpTimer.Reset();
	JumpTimer.Start();

	Name = "Dynamic Character Controller";
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

void G1I::DynamicCharacterController::BeforeDraw()
{
	if (mouse_locked)
	{
		auto ychange = mResources->MousePosition->y != *mResources->CenterY, xchange = mResources->MousePosition->x != *mResources->CenterX;
		if (ychange)
		{
			Cam->Pitch(float(float(mResources->MousePosition->y - *mResources->CenterY) * multiplier * (*mResources->InverseWindowHeight)));
		}

		if (xchange)
		{
			Cam->RotateY(float(float(mResources->MousePosition->x - *mResources->CenterX) * multiplier * (*mResources->InverseWindowWidth)));
		}

		if (xchange || ychange)
		{
			SDL_WarpMouseInWindow(mResources->Window, *mResources->CenterX, *mResources->CenterY);
		}
	}

	auto phys = Container->GetPhysicsWorld();

	if (auto dynamicsworld = phys.lock())
	{
		OnGround(dynamicsworld.get());
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
				RigidBody->setDamping(2.f, RigidBody->getAngularDamping());
			}


			if (jump && jumpcooldown > JumpCooldown)
			{
				RigidBody->setDamping(0.f, RigidBody->getAngularDamping());
				RigidBody->applyCentralImpulse({ 0.f, 50.f, 0.f });
				DINFO("Applied jump impulse, Rigid body is: " + (RigidBody->isActive() ? "Active" : "Inactive") + " and has velocity of: " + AsString(floaty3(RigidBody->getLinearVelocity())));
				JumpTimer.Reset();
			}
		}
		else
		{
			RigidBody->setDamping(0.f, RigidBody->getAngularDamping());
			if (forwardness || rightness)
			{
				DINFO("Applying mid-air force");
				auto camforward = Cam->GetLook();
				auto camright = Cam->GetRight();
				btVector3 forwardv(camforward.x, 0.f, camforward.z);
				btVector3 rightv(camright.x, 0.f, camright.z);
				btVector3 target((forwardv * -forwardness) + (rightv * rightness));
				target.normalize();

				btVector3 vel = RigidBody->getLinearVelocity();
				vel.setY(0.f);
				if (!vel.isZero())
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

void G1I::DynamicCharacterController::AfterDraw()
{
}

bool G1I::DynamicCharacterController::Receive(Events::IEvent * event)
{
	if (auto *win_focus = Events::ConvertEvent<Event::WindowFocusEvent>(event))
	{
		mouse_locked &= win_focus->GainedFocus;
		rightness = forwardness = 0.f;
		forward = back = right = left = false;
		jump = false;

		return Events::RelevantEvent;
	}
	else if (auto *afe = Events::ConvertEvent<Event::AfterPhysicsEvent>(event))
	{
		floaty3 ass(RigidBody->getWorldTransform().getOrigin());
		ass.y = ass.y + 0.25f * CapsuleHeight;
		ass.z = -ass.z; // Does a lot
		Cam->SetPosition(ass);

		static int CameraUpdateThing = 0;

		++CameraUpdateThing;

		if ((CameraUpdateThing % 20) == 0)
		{
			floaty3 Pos = Cam->GetPosition();
			DINFO("Updating Camera with look: " + AsString(Cam->GetLook()) + " and ass: " + AsString(ass) + " with Camera Position: " + AsString(Cam->GetPosition()));
		}

		return Events::RelevantEvent;
	}
	return Events::IrrelevantEvent;
}

bool G1I::DynamicCharacterController::Receive(Event::KeyInput * key)
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

bool G1I::DynamicCharacterController::Receive(Event::MouseMove * e)
{
	(void)e;
	return true;
}

bool G1I::DynamicCharacterController::Receive(Event::MouseButton * e)
{
	if (e->Button == RMB)
	{
		mouse_locked = e->State;
	}
	return true;
}

bool G1I::DynamicCharacterController::Receive(Event::MouseWheelButton * e)
{
	(void)e;
	return false;
}

bool G1I::DynamicCharacterController::Receive(Event::MouseWheel * e)
{
	multiplier += float(0.1f * e->Scrollage);
	multiplier = fmaxf(0.1f, fminf(5.f, multiplier));
	return true;
}

bool G1I::DynamicCharacterController::Receive(Event::ResizeEvent * e)
{
	(void)e;
	Cam->SetLens(Math::QuatPiF, float(*mResources->AspectRatio), IdealNearPlane, IdealFarPlane);
	return true;
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
			btScalar dot = cp.m_normalWorldOnB.y();
			if (dot < maxNormalDot)
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

bool G1I::DynamicCharacterController::OnGround(btDynamicsWorld *world)
{
	GroundCallback callback(*RigidBody, btScalar(0.25f));
	world->contactTest(RigidBody.get(), callback);
	groundy = callback.OnGround;
	if (callback.foundDot < debugDot - 0.00006103515625f || callback.foundDot > debugDot + 0.00006103515625f)
	{
		DINFO("Dot product changed to: '" + std::to_string(callback.foundDot) + "' with normal as (" + std::to_string(callback.foundNormal.x()) + ", " + std::to_string(callback.foundNormal.y()) + ", " + std::to_string(callback.foundNormal.z()) + ")");
		debugDot = callback.foundDot;
	}
	return groundy;
}

#pragma warning(default:4100)

void G1I::PhysicsShape::getWorldTransform(btTransform & t) const
{
	t.setFromOpenGLMatrix(World.ma);
}

void G1I::PhysicsShape::setWorldTransform(const btTransform & t)
{
	t.getOpenGLMatrix(World.ma);
}

floaty3 G1I::PhysicsShape::GetPosition()
{
	return { World.dx, World.dy, World.dz };
}

G1I::EnviroShape::EnviroShape(G1::IGSpace *container, CommonResources *resources, std::string name) : IShape(container), FullResourceHolder(resources), PhysicsRenderShape(container, std::vector<FullVertex>(EnviroThings::Vertices.cbegin(), EnviroThings::Vertices.cend()), std::vector<unsigned int>(EnviroThings::Indices.cbegin(), EnviroThings::Indices.cend()), EnviroThings::Mat, Matrixy4x4::Translate(EnviroThings::Pos))
{
	Name = name;

	if (!Shape)
		Shape = std::make_unique<btBoxShape>(btVector3(btScalar(5.f * EnviroThings::scale), btScalar(0.1f), btScalar(5.f * EnviroThings::scale)));

	RigidBody = std::make_shared<btRigidBody>(btRigidBody::btRigidBodyConstructionInfo(btScalar(0.f), this, Shape.get()));

	Container->RequestPhysicsCall(RigidBody);
}

G1I::CubeShape::CubeShape(G1::IGSpace *container, CommonResources *resources, floaty3 extents, floaty3 pos, bool dynamic) : IShape(container, "CubeShape"), PhysicsShape(resources, pos), Extents(extents), Dynamic(dynamic)
{
	bShape = std::make_unique<btBoxShape>(btVector3(extents.x, extents.y, extents.z));

	if (Dynamic)
	{
		btVector3 localInertia;
		bShape->calculateLocalInertia(btScalar(1.f), localInertia);

		RigidBody = std::make_shared<btRigidBody>(btRigidBody::btRigidBodyConstructionInfo(btScalar(1.f), this, bShape.get(), localInertia));
	}
	else
	{
		RigidBody = std::make_shared<btRigidBody>(btRigidBody::btRigidBodyConstructionInfo(btScalar(0.f), this, bShape.get()));
	}

	Container->RequestPhysicsCall(RigidBody);
}

G1I::CubePRShape::CubePRShape(G1::IGSpace * container, CommonResources * resources, std::string geodatafile, floaty3 extents, bool dynamic, float mass) : PhysicsRenderShape(container, geodatafile), IShape(container, "Random Imported 'Cube'"), FullResourceHolder(resources), Extents(extents), Dynamic(dynamic)
{
	bShape = std::make_unique<btBoxShape>(btVector3(Extents.x, Extents.y, Extents.z));

	if (Dynamic)
	{
		btVector3 localInertia;
		bShape->calculateLocalInertia(btScalar(1.f), localInertia);

		RigidBody = std::make_shared<btRigidBody>(btRigidBody::btRigidBodyConstructionInfo(btScalar(mass), this, bShape.get(), localInertia));
	}
	else
	{
		RigidBody = std::make_shared<btRigidBody>(btRigidBody::btRigidBodyConstructionInfo(btScalar(0.f), this, bShape.get()));
	}

	Container->RequestPhysicsCall(RigidBody);
}

G1I::CubeyShape::CubeyShape(G1::IGSpace * container, CommonResources * resources, Material mat, floaty3 extents, G1I::bodytype dynamic, btCollisionShape *shape, floaty3 pos, int CollisionGroup, int collisionmask) : CubeyShape(container, resources, mat, extents, dynamic, shape, Matrixy4x4::Translate(pos), CollisionGroup, collisionmask) {}

G1I::CubeyShape::CubeyShape(G1::IGSpace * container, CommonResources * resources, Material mat, floaty3 extents, bodytype dynamic, btCollisionShape * shape, Matrixy4x4 trans, int CollisionGroup, int CollisionMask) : FullResourceHolder(resources), IShape(container, "CubeShape"), PhysicsRenderShape(container, TransformVertices(extents), std::vector<unsigned int>(CubeIndices.cbegin(), CubeIndices.cend()), mat, trans), Extents(extents), BodyType(dynamic)
{
	if (shape == nullptr)
	{
		bShape = std::make_unique<btBoxShape>(btVector3(extents.x, extents.y, extents.z));
		shape = bShape.get();
	}

	if (BodyType == bodytype::Dynamic)
	{
		btVector3 localInertia;
		shape->calculateLocalInertia(btScalar(1.f), localInertia);

		RigidBody = std::make_shared<btRigidBody>(btRigidBody::btRigidBodyConstructionInfo(btScalar(1.f), this, shape, localInertia));

		if (CollisionGroup == btBroadphaseProxy::StaticFilter)
		{
			DWARNING("Dynamic object being created with static group, changed to Dynamic");
			CollisionGroup = btBroadphaseProxy::DefaultFilter;
		}
		if (CollisionMask == (btBroadphaseProxy::AllFilter ^ btBroadphaseProxy::StaticFilter))
		{
			DWARNING("Dynamic object being created with All - Static mask, changed to All");
			CollisionMask = btBroadphaseProxy::AllFilter;
		}
	}
	else if (bodytype::Static)
	{
		RigidBody = std::make_shared<btRigidBody>(btRigidBody::btRigidBodyConstructionInfo(btScalar(0.f), this, shape));
	}
	else
	{
		// Kinematic
		RigidBody = std::make_shared<btRigidBody>(btRigidBody::btRigidBodyConstructionInfo(btScalar(0.f), this, shape));

		RigidBody->setCollisionFlags(RigidBody->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT);
		RigidBody->setActivationState(DISABLE_DEACTIVATION);
	}

	Container->RequestPhysicsCall(RigidBody, CollisionGroup, CollisionMask);
}

G1I::CubeyShape::CubeyShape(G1::IGSpace * container, CommonResources * resources, std::shared_ptr<Material> mat, floaty3 extents, bodytype dynamic, btCollisionShape * shape, Matrixy4x4 trans, int CollisionGroup, int CollisionMask) : FullResourceHolder(resources), IShape(container, "CubeShape"), PhysicsRenderShape(container, TransformVertices(extents), std::vector<unsigned int>(CubeIndices.cbegin(), CubeIndices.cend()), mat, trans), Extents(extents), BodyType(dynamic)
{
	if (shape == nullptr)
	{
		bShape = std::make_unique<btBoxShape>(btVector3(extents.x, extents.y, extents.z));
		shape = bShape.get();
	}

	if (BodyType == bodytype::Dynamic)
	{
		btVector3 localInertia;
		shape->calculateLocalInertia(btScalar(1.f), localInertia);

		RigidBody = std::make_shared<btRigidBody>(btRigidBody::btRigidBodyConstructionInfo(btScalar(1.f), this, shape, localInertia));

		if (CollisionGroup == btBroadphaseProxy::StaticFilter)
		{
			DWARNING("Dynamic object being created with static group, changed to Dynamic");
			CollisionGroup = btBroadphaseProxy::DefaultFilter;
		}
		if (CollisionMask == (btBroadphaseProxy::AllFilter ^ btBroadphaseProxy::StaticFilter))
		{
			DWARNING("Dynamic object being created with All - Static mask, changed to All");
			CollisionMask = btBroadphaseProxy::AllFilter;
		}
	}
	else if (bodytype::Static)
	{
		RigidBody = std::make_shared<btRigidBody>(btRigidBody::btRigidBodyConstructionInfo(btScalar(0.f), this, shape));
	}
	else
	{
		// Kinematic
		RigidBody = std::make_shared<btRigidBody>(btRigidBody::btRigidBodyConstructionInfo(btScalar(0.f), this, shape));

		RigidBody->setCollisionFlags(RigidBody->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT);
		RigidBody->setActivationState(DISABLE_DEACTIVATION);
	}

	Container->RequestPhysicsCall(RigidBody, CollisionGroup, CollisionMask);
}

G1I::CubeyShape::CubeyShape(G1::IGSpace * container, CommonResources * resources, Drawing::DrawCall call, bodytype dynamic, btCollisionShape * shape, Matrixy4x4 trans, int CollisionGroup, int CollisionMask) : FullResourceHolder(resources), IShape(container, "Instanced Cube Shape"), PhysicsRenderShape(container, call), BodyType(dynamic)
{
	World = trans;

	assert(shape);

	if (BodyType == bodytype::Dynamic)
	{
		btVector3 localInertia;
		bShape->calculateLocalInertia(btScalar(1.f), localInertia);

		RigidBody = std::make_shared<btRigidBody>(btRigidBody::btRigidBodyConstructionInfo(btScalar(1.f), this, shape, localInertia));

		if (CollisionGroup == btBroadphaseProxy::StaticFilter)
		{
			DWARNING("Dynamic object being created with static group, changed to Dynamic");
			CollisionGroup = btBroadphaseProxy::DefaultFilter;
		}
		if (CollisionMask == (btBroadphaseProxy::AllFilter ^ btBroadphaseProxy::StaticFilter))
		{
			DWARNING("Dynamic object being created with All - Static mask, changed to All");
			CollisionMask = btBroadphaseProxy::AllFilter;
		}
	}
	else if (bodytype::Static)
	{
		RigidBody = std::make_shared<btRigidBody>(btRigidBody::btRigidBodyConstructionInfo(btScalar(0.f), this, shape));
	}
	else
	{
		// Kinematic
		RigidBody = std::make_shared<btRigidBody>(btRigidBody::btRigidBodyConstructionInfo(btScalar(0.f), this, shape));

		RigidBody->setCollisionFlags(RigidBody->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT);
		RigidBody->setActivationState(DISABLE_DEACTIVATION);
	}

	Container->RequestPhysicsCall(RigidBody, CollisionGroup, CollisionMask);
}

G1I::CubeyShape::~CubeyShape()
{
}

void G1I::CubeyShape::SetPosition(floaty3 pos)
{
	World.dx = pos.x;
	World.dy = pos.y;
	World.dz = pos.z;
	btTransform trans;
	trans.setFromOpenGLMatrix(World.ma);
	RigidBody->setWorldTransform(trans);
}

void G1I::CubeyShape::SetRotation(floaty3 rot)
{
	Matrixy3x3 RotMat = Matrixy3x3::RotationR(rot);
	World.m11 = RotMat.m11;
	World.m12 = RotMat.m12;
	World.m13 = RotMat.m13;
	World.m21 = RotMat.m21;
	World.m22 = RotMat.m22;
	World.m23 = RotMat.m23;
	World.m31 = RotMat.m31;
	World.m32 = RotMat.m32;
	World.m33 = RotMat.m33;
}

void G1I::CubeyShape::Disable()
{
	if (!Enabled()) // > Kinematic means its one of the Removed types meaning no work to do
		return;
	Container->RequestPhysicsRemoval(RigidBody.get());
	BodyType = BodyType << 3;
}

void G1I::CubeyShape::Enable(int collisiongroup, int collisionmask)
{
	if (Enabled()) // < Removed_Static means its one of the Activated types meaning no work to do
		return;
	Container->RequestPhysicsCall(RigidBody,collisiongroup,collisionmask);
	BodyType = BodyType >> 3;
}

bool G1I::CubeyShape::Enabled()
{
	return BodyType < Removed_Static;
}

void G1I::CubeyShape::SetUserPointer(void * p)
{
	RigidBody->setUserPointer(p);
}

void G1I::CubeyShape::SetUserIndex(int i)
{
	RigidBody->setUserIndex(i);
}

void G1I::CubeyShape::SetUserIndex2(int i)
{
	RigidBody->setUserIndex2(i);
}

std::vector<FullVertex> G1I::TransformVertices(floaty3 extents)
{
	std::vector<FullVertex> vertices{ CubeVertices.cbegin(), CubeVertices.cend() };

	Matrixy3x3 scale = Matrixy3x3::Scale(extents.x / 0.5f, extents.y / 0.5f, extents.z / 0.5f);

	for (size_t i = vertices.size(); i-- > 0; )
	{
		vertices[i].PosL = scale.Transform(vertices[i].PosL);
	}

	return vertices;
}

G1I::RawRendyShape::RawRendyShape(G1::IShapeThings things, Drawing::DrawCall call) : FullResourceHolder(things.Resources), IShape(things), key(mResources->Ren3->AddDrawCall(call))
{
	mResources->Ren3->GetDrawCall(key)->DebugString = GetName();
}

G1I::RawRendyShape::~RawRendyShape()
{
	if (key)
		mResources->Ren3->RemoveDrawCall(key);
}

Drawing::DrawCall * G1I::RawRendyShape::GetCall()
{
	if (key)
		return mResources->Ren3->GetDrawCall(key);
	return nullptr;
}

G1I::PhysicsBody::PhysicsBody(G1::IShapeThings things, G1I::PhysicsThing pthings) : FullResourceHolder(things.Resources), IShape(things), PhysicsShape(pthings.Trans), Info(pthings)
{
	if (Info.Dynamic)
	{
		btVector3 localInertia;
		Info.Shape->calculateLocalInertia(btScalar(Info.Mass), localInertia);

		Body = std::make_shared<btRigidBody>(btRigidBody::btRigidBodyConstructionInfo(btScalar(Info.Mass), this, Info.Shape.get(), localInertia));
	}
	else
	{
		Body = std::make_shared<btRigidBody>(btRigidBody::btRigidBodyConstructionInfo(btScalar(0.f), this, Info.Shape.get()));
	}

	Body->setFriction(Info.Friction);

	Container->RequestPhysicsCall(Body, Info.CollisionGroup, Info.CollisionMask);
}

std::shared_ptr<btCollisionShape> G1I::PhysicsBody::GetShape()
{
	return Info.Shape;
}

G1I::RawMatrixRendyShape::RawMatrixRendyShape(G1::IShapeThings things, Drawing::DrawCall call, Matrixy4x4 mat) : FullResourceHolder(things.Resources), RawRendyShape(things, call), World(mat)
{
	this->GetCall()->Matrix = &World;
}

G1I::BvhPhysicsBody::BvhPhysicsBody(G1::IShapeThings tings, PhysicsThing pting, BvhThings btings) : IShape(tings), FullResourceHolder(tings.Resources), m_PhysicsInfo(pting), m_BvhInfo(btings), m_MeshData(std::make_shared<btTriangleIndexVertexArray>())
{
	assert(!pting.Shape && "Do not supply a shape to a BvhPhysicsBody it creates one with the mesh you supply");
	btIndexedMesh mesh;
	mesh.m_numTriangles = (int)m_BvhInfo.Indices.size() / 3;
	mesh.m_triangleIndexBase = (const unsigned char*)m_BvhInfo.Indices.data();
	mesh.m_triangleIndexStride = sizeof(int) * 3;
	mesh.m_numVertices = (int)m_BvhInfo.Vertices.size();
	mesh.m_vertexBase = (const unsigned char*) m_BvhInfo.Vertices.data();
	mesh.m_vertexStride = sizeof(floaty3);
	mesh.m_vertexType = PHY_FLOAT;
	m_MeshData->addIndexedMesh(mesh, PHY_INTEGER);

	World = pting.Trans;

	m_PhysicsInfo.Shape = std::make_shared<btBvhTriangleMeshShape>(m_MeshData.get(), true);

	if (m_PhysicsInfo.Dynamic)
	{
		btVector3 localInertia;
		m_PhysicsInfo.Shape->calculateLocalInertia(btScalar(m_PhysicsInfo.Mass), localInertia);

		m_Body = std::make_shared<btRigidBody>(btRigidBody::btRigidBodyConstructionInfo(btScalar(m_PhysicsInfo.Mass), this, m_PhysicsInfo.Shape.get(), localInertia));
	}
	else
	{
		m_Body = std::make_shared<btRigidBody>(btRigidBody::btRigidBodyConstructionInfo(btScalar(0.f), this, m_PhysicsInfo.Shape.get()));
	}

	m_Body->setFriction(m_PhysicsInfo.Friction);

	Container->RequestPhysicsCall(m_Body, pting.CollisionGroup, pting.CollisionMask);
}

std::shared_ptr<btTriangleIndexVertexArray> G1I::BvhPhysicsBody::GetTriangleIndexVertexArray()
{
	return m_MeshData;
}

G1I::LightShape::LightShape(G1::IShapeThings ting, Light light) : IShape(ting), FullResourceHolder(ting.Resources), m_LightIndex(GetFreeIndex())
{
	auto *mlight = GetLight();
	(*mlight) = light;
}

G1I::LightShape::~LightShape()
{
	if (m_LightIndex != (size_t)-1)
	{
		TakenLights[m_LightIndex] = false;
		auto *light = mResources->Ren3->GetLight((unsigned int)m_LightIndex);
		if (light)
		{
			light->Enabled = false;
			light->Color = { 0.f, 0.f, 0.f };
			light->Range = 0.f;
		}
	}
}

void G1I::LightShape::BeforeDraw()
{
}

void G1I::LightShape::AfterDraw()
{
}

Light *G1I::LightShape::GetLight()
{
	return mResources->Ren3v2->GetLight(m_LightIndex);
}

size_t G1I::LightShape::GetFreeIndex()
{
	for (size_t i = 0; i < LIGHT_COUNT; ++i)
	{
		if (!TakenLights[i])
		{
			TakenLights[i] = true;
			return i;
		}
	}
	throw NoFreeLightException();
}

G1I::ProfilerShape::ProfilerShape(G1::IShapeThings ting1, ProfilerThings ting2) : FullResourceHolder(ting1.Resources), IShape(ting1), m_Info(ting2)
{
	if (ting2.Running)
	{
		m_Timer.Reset();
		m_Timer.Start();
		m_StartID = *mResources->UpdateID;
	}
	m_DeltaTimes.reserve(10 * 60 + 3);

	mResources->Request->Add(this);
}

void G1I::ProfilerShape::BeforeDraw()
{
	if (m_Info.Running)
	{
		m_DeltaTimes.push_back(*mResources->DeltaTime);
		m_Timer.Tick();
		if (m_Timer.TotalTime() > m_Info.ProfileTime)
		{
			ReportProfiling();
			m_Info.Running = false;
		}
	}
}

void G1I::ProfilerShape::AfterDraw()
{
}

void G1I::ProfilerShape::Restart()
{
	if (m_Info.Running)
	{
		m_Timer.Tick();
		ReportProfiling(m_Timer.TotalTime());
	}
	m_Timer.Reset();
	m_Timer.Start();
	m_Info.Running = true;
	m_DeltaTimes.clear();
	m_DeltaTimes.reserve((size_t)(m_Info.ProfileTime * 60.0));
	m_StartID = *mResources->UpdateID;
#ifdef EC_PROFILE
	mResources->Profile->Start();
#endif
}

Debug::DebugReturn G1I::ProfilerShape::Request(Requests::Request &req)
{
	if (req.Name == "StartSimpleProfile")
	{
		Restart();
		return true;
	}
	return false;
}

void G1I::ProfilerShape::ReportProfiling()
{
	ReportProfiling(m_Info.ProfileTime);
}

void G1I::ProfilerShape::ReportProfiling(double time)
{
#ifndef EC_PROFILE
	(void)time;
#else
	if (!m_Info.Running)
		return;

	size_t current = *mResources->UpdateID;
	double framecount = static_cast<double>(current - m_StartID);
	double fps = framecount / time;

	double average = 0.0;
	for (auto &delta : m_DeltaTimes)
		average += delta;

	average /= (double)m_DeltaTimes.size();

	double rangelow = 2000.0, rangehigh = 0.0;
	for (auto &delta : m_DeltaTimes)
	{
		rangelow = fmin(delta, rangelow);
		rangehigh = fmax(delta, rangehigh);
	}
	
	DINFO("--------------Profiling-----------------");
	DINFO("Profiler ran for " + std::to_string(time) + " seconds");
	DINFO(std::to_string(framecount) + " frames were completed");
	DINFO("With an average delta time of " + std::to_string(average));
	DINFO("Gaining " + std::to_string(fps) + " Frames per second");
	DINFO("With a range of " + std::to_string(rangelow) + "-" + std::to_string(rangehigh));
	DINFO("Querying ProfilerMcGee...");
	
	const auto &stats = mResources->Profile->Finish(true);

	DINFO("Writing frames to file 'profile.txt'");

	BigBoiStats::WriteToFile(stats, "profile.txt");

	DINFO("-----------End Profiling----------------");
#endif
}

G1I::GLRen2TestShape::GLRen2TestShape(G1::IShapeThings tings) 
	: G1::IShape(tings.WithName("GLRen2TestShape"))
{
	std::shared_ptr<Drawing::Mesh> epico_mesh = std::make_shared<Drawing::Mesh>(Drawing::CreateCubeMesh(), Drawing::MeshStorageType::STATIC_BUFFER);

	std::shared_ptr<Drawing::Material> epico_mat = Drawing::MaterialStore::Instance().GetMaterial("example-material");

	m_DrawCallKey = tings.Resources->Ren3v2->SubmitDrawCall(Drawing::DrawCallv2{ std::move(epico_mesh), std::move(epico_mat), std::make_shared<Matrixy4x4>(Matrixy4x4::Translate(0.f, 5.f, 1.f)), "GLren2 Test", true });
}
