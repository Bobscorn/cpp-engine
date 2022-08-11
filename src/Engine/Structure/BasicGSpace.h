#pragma once

#include "Helpers/VectorHelper.h"
#include "Helpers/StringHelper.h"
#include "Helpers/MathHelper.h"

#include "../Drawing/Graphics1.h"
#include "PhysicsSpace.h"

#include "Systems/Requests/Requester.h"

namespace G1I
{
	using namespace Pointer;
	
	struct BasicDebugDrawer : public btIDebugDraw
	{
	private:
		GLProgram Program;
		GLBuffer VertexBuffer;
		GLBuffer MatrixBuffer;
		GLVertexArray VAO;
		GLuint MatBufBinding;
		
		struct Line
		{
			// Aligned along 16 byte boundaries to ensure it matches gpu alignment
			floaty3 A;
			floaty3 Color1;
			floaty3 B;
			floaty3 Color2;

			Line(floaty3 a, floaty3 b) : A(a), B(b) {}
			Line(const btVector3& a, const btVector3& b, const btVector3& color) : A(a.x(), a.y(), a.z()), B(b.x(), b.y(), b.z()), Color1(color.x(), color.y(), color.z()), Color2(color.x(), color.y(), color.z()) {}
			Line(const btVector3& a, const btVector3& b, const btVector3& color1, const btVector3& color2) : A(a.x(), a.y(), a.z()), B(b.x(), b.y(), b.z()), Color1(color1.x(), color1.y(), color1.z()), Color2(color2.x(), color2.y(), color2.z())  {}

			inline bool operator==(const Line& other) const { return A == other.A && B == other.B && Color1 == other.Color1 && Color2 == other.Color2; }
			inline bool operator!=(const Line& other) const { return !(*this == other); }
		};

		std::vector<Line> CurrentBuffer;
		std::vector<Line> OldBuffer;
		int debugmode; // used by bullet I think
		IRen3D *Ren = nullptr;
	public:
		BasicDebugDrawer(CommonResources *resources);

		void drawLine(const btVector3& from, const btVector3& to, const btVector3& color) override;
		void drawLine(const btVector3& from, const btVector3& to, const btVector3& color1, const btVector3& color2) override;
		void drawContactPoint(const btVector3& PointOnB, const btVector3& normalOnB, btScalar distance, int lifeTime, const btVector3& color) override { (void)PointOnB; (void)normalOnB; (void)distance; (void)lifeTime; (void)color; }
		inline void reportErrorWarning(const char* warningString) override { DERROR(warningString); }
		void draw3dText(const btVector3& location, const char* textString) override { (void)location; (void)textString; }
		inline void setDebugMode(int debugMode) override { debugmode = debugMode; }
		inline int getDebugMode() const override { return debugmode; }

		bool Initialize(CommonResources *resources);
		bool Draw(Matrixy4x4 ViewProj);
	};

#pragma warning(disable:4250)
	struct BasicGSpace : virtual G1I::PhysicsSpace, virtual G1::IGSpace, Events::IEventListenerT<Events::ResizeWindowEvent>
	{
	protected:

		Camera Cam;

		BasicDebugDrawer debugdrawer;
		bool DebugDrawing = false;
		bool RealDrawing = true;
	public:
		observing_ptr<Camera> ExternalCamera;

		BasicGSpace(CommonResources *resources, std::string FileName);
		BasicGSpace(CommonResources *resources) : IGSpace(resources), FullResourceHolder(resources), debugdrawer(resources) {
			Cam.SetLens(Math::HalfPiF, (float)*mResources->AspectRatio, IdealNearPlane, IdealFarPlane);
			Cam.SetPosLookUp({ 0.f, 0.f, 0.f }, { 0.f, 0.f, -1.f }, { 0.f, 1.f, 0.f });
			this->DynamicsWorld->setDebugDrawer(&debugdrawer);
			resources->Request->Add(this);
		}; // TODO: implement real constructor
		virtual ~BasicGSpace() { this->Root.children.clear(); };

		void ImportFile(std::string FileName);

		inline bool Receive(Events::IEvent *event) override { return false; }
		inline bool Receive(Event::ResizeEvent *event) override;

		virtual void BeforeDrawI() override;
		virtual void DrawI() override;
		virtual void AfterDrawI() override;

		void SetCamera(const observing_ptr<Camera>& c) override;
		Camera *GetCamera() override;
		const Camera *GetCamera() const override;
		bool CameraIsExternal() const override;

		using G1I::PhysicsSpace::AddCollisionObject;
		using G1I::PhysicsSpace::AddRigidBody;
		using G1I::PhysicsSpace::RemoveCollisionObject;
		using G1I::PhysicsSpace::RemoveRigidBody;
		using G1I::PhysicsSpace::SimulatePhysics;

		virtual Debug::DebugReturn Request(Requests::Request& action) override;
		inline virtual std::string GetName() const override { return "BasicGSpace"; }
	};
#pragma warning(default:4250)

	
}