//#include "Thing.h"
#include "BasicGSpace.h"

#include "Helpers/IteratorHelper.h"
#include "Helpers/DebugHelper.h"
#include "Helpers/ProfileHelper.h"

#include "Drawing/Graphics3D.h"
#include "Drawing/IRen3D.h"
#include "Systems/Events/PhysicsEvent.h"
#include "Systems/Events/Events.h"

G1I::BasicGSpace::BasicGSpace(CommonResources *resources, std::string FileName) : IGSpace(resources), FullResourceHolder(resources), debugdrawer(resources)
{
	ImportFile(FileName);
}

void G1I::BasicGSpace::ImportFile(std::string FileName)
{

}

inline bool G1I::BasicGSpace::Receive(Event::ResizeEvent * event)
{
	(void)event;
	Cam.SetLens(Math::HalfPiF, *mResources->AspectRatio, IdealNearPlane, IdealFarPlane);
	return true;
}

void G1I::BasicGSpace::BeforeDrawI()
{
}

void G1I::BasicGSpace::DrawI()
{
	PROFILE_PUSH("BasicGSpace Draw");
	if (RealDrawing)
	{
		if (ExternalCamera)
		{
			ExternalCamera->UpdateViewMatrix();
			mResources->Ren3->DrawCalls(ExternalCamera->View(), ExternalCamera->Proj(), ExternalCamera->CreateOptimizedFrustum());
		}
		else
		{
			Cam.UpdateViewMatrix();
			mResources->Ren3->DrawCalls(Cam.View(), Cam.Proj(), Cam.CreateOptimizedFrustum());
		}
	}

	if (DebugDrawing)
	{
		DynamicsWorld->setDebugDrawer(&debugdrawer);
		DynamicsWorld->debugDrawWorld();
		debugdrawer.drawLine({ 0.f, 1.5f, 0.f }, { 1.f, 1.5f, 0.f }, { 1.f, 0.f, 0.f });
		debugdrawer.drawLine({ 0.f, 1.5f, 0.f }, { 0.f, 2.5f, 0.f }, { 0.f, 1.f, 0.f });
		debugdrawer.drawLine({ 0.f, 1.5f, 0.f }, { 0.f, 1.5f, 1.f }, { 0.f, 0.f, 1.f });
		debugdrawer.Draw((ExternalCamera ? ExternalCamera->ViewProj() : Cam.ViewProj()));
	}
	PROFILE_POP();
}

#pragma warning(suppress:4100)
void G1I::BasicGSpace::AfterDrawI()
{
	PROFILE_PUSH("BasicGSpace AfterDraw");
	PROFILE_PUSH("PhysicsCull");
	this->PhysicsCull(this->CollisionObjects, this->RigidBodies);
	PROFILE_POP();

	PROFILE_PUSH("BeforePhysicsEvent");
	Event::BeforePhysicsEvent bfe(this->DynamicsWorld.get(), *mResources->UpdateID);
	mResources->Event->Send(&bfe);
	PROFILE_POP();

	this->DoPhysics();

	PROFILE_PUSH("AfterPhyicsEvent");
	Event::AfterPhysicsEvent afe(this->DynamicsWorld.get(), *mResources->UpdateID);
#ifdef EC_PROFILE
	mResources->Event->Send(&afe, mResources->Profile);
#else
	mResources->Event->Send(&afe);
#endif
	PROFILE_POP();
	PROFILE_POP();
}

void G1I::BasicGSpace::SetCamera(const Pointer::observing_ptr<Camera>& c)
{
	ExternalCamera = c;
}

Camera * G1I::BasicGSpace::GetCamera()
{
	if (ExternalCamera)
		return ExternalCamera.get();
	else
		return &Cam;
}

const Camera * G1I::BasicGSpace::GetCamera() const
{
	if (ExternalCamera)
		return ExternalCamera.get();
	else
		return &Cam;
}

bool G1I::BasicGSpace::CameraIsExternal() const
{
	return ExternalCamera.get();
}

/*
void G1I::BasicGSpace::RequestGeometryCall(const std::vector<Geometry::Vertex>& Vertices, const std::vector<Geometry::Index>& Indices, Pointer::selfish_ptr<GI::RangeIndex>& geoindexholder,
	Pointer::selfish_ptr<DrawCall>& drawholder)
{
	if (!drawholder)
		drawholder = Pointer::make_selfish<DrawCall>();

	Geometry.InsertGeometry(Vertices, Indices, geoindexholder);

	drawholder->SetIndices(observing_ptr<GI::RangeIndex>(geoindexholder));

	BasicDrawGroup.emplace_back(observing_ptr<DrawCall>(drawholder));
	auto enabled = drawholder->IsEnabled();
	auto indices = drawholder->Indices();
	if (enabled)
		DINFO("An enabled DrawCall has been added with '" + std::to_wstring(indices.VertexCount) + "' vertices and '" + std::to_wstring(indices.IndexCount) + "' indices");
	else
		DINFO("A disabled DrawCall has been added with '" + std::to_wstring(indices.VertexCount) + "' vertices and '" + std::to_wstring(indices.IndexCount) + "' indices");
	
	DebugVersion.push_back(Drawing::DrawCallDebugInfo{ enabled, indices });
}*/

Debug::DebugReturn G1I::BasicGSpace::Request(Requests::Request& action)
{
	if (action.Name == "ToggleDebugDraw")
	{
		DebugDrawing = !DebugDrawing;
		DINFO("Debug Drawing has been toggled");
		return true;
	}
	else if (action.Name == "ToggleRealDraw")
	{
		RealDrawing = !RealDrawing;
		DINFO("Real Drawing has been toggled");
		return true;
	}
	else
		return PhysicsSpace::Request(action);
}


G1I::BasicDebugDrawer::BasicDebugDrawer(CommonResources *resources)
	: Ren(resources->Ren3)
{
	Program = GLProgram({ {"Shaders/bulletdebug.glvs", GL_VERTEX_SHADER} , {"Shaders/bulletdebug.glfs", GL_FRAGMENT_SHADER} });

	GLuint matbuf = 0;
	glGenBuffers(1, &matbuf);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, matbuf);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(Matrixy4x4), 0, GL_STREAM_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	MatrixBuffer = matbuf;

	// Vertex buffer created in Draw method

	GLuint vao = 0;
	glGenVertexArrays(1, &vao);
	VAO = vao;
}

void G1I::BasicDebugDrawer::drawLine(const btVector3 & from, const btVector3 & to, const btVector3 & color)
{
	drawLine(from, to, color, color);
}

void G1I::BasicDebugDrawer::drawLine(const btVector3 & from, const btVector3 & to, const btVector3 & color1, const btVector3 & color2)
{
	OldBuffer.emplace_back(from, to, color1, color2);
}

bool G1I::BasicDebugDrawer::Draw(Matrixy4x4 ViewProj)
{
	if (OldBuffer == CurrentBuffer)
	{
		// No copy required
		OldBuffer.clear();
	}
	else
	{
		if (OldBuffer.empty())
			CurrentBuffer.clear();
		else
		{
			// Copy required
			if (OldBuffer.size() != CurrentBuffer.size())
			{
				GLuint vertexbuffer = 0;
				glGenBuffers(1, &vertexbuffer);
				glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
				glBufferData(GL_ARRAY_BUFFER, sizeof(Line) * OldBuffer.size(), OldBuffer.data(), GL_DYNAMIC_DRAW);
				glBindVertexArray(VAO.Get());
				glEnableVertexAttribArray(0);
				glEnableVertexAttribArray(1);
				glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Line) / 2, 0);
				glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Line) / 2, (const void*)(sizeof(float) * 3));
				glBindBuffer(GL_ARRAY_BUFFER, 0);
				glBindVertexArray(0);

				VertexBuffer = vertexbuffer;
			}
			else
			{
				glBindBuffer(GL_ARRAY_BUFFER, VertexBuffer.Get());
				void *dat = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
				memcpy(dat, OldBuffer.data(), sizeof(Line) * OldBuffer.size());
				glUnmapBuffer(GL_ARRAY_BUFFER);
			}

			CurrentBuffer = std::move(OldBuffer);
			OldBuffer.clear();
		}
	}

	if (CurrentBuffer.empty())
		return true;

	glBindBuffer(GL_UNIFORM_BUFFER, MatrixBuffer.Get());
	void *dat = glMapBuffer(GL_UNIFORM_BUFFER, GL_WRITE_ONLY);
	memcpy(dat, &ViewProj, sizeof(Matrixy4x4));
	glUnmapBuffer(GL_UNIFORM_BUFFER);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	glBindVertexArray(VAO.Get());
	glUseProgram(Program.Get());

	glBindBuffer(GL_ARRAY_BUFFER, VertexBuffer.Get());

	glDrawArrays(GL_LINES, 0, (GLsizei)CurrentBuffer.size() * 2);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glUseProgram(0);
	glBindVertexArray(0);

	Ren->DidSomething();

	return true;
}