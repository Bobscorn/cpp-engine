#include "VoxelScene.h"

#include "Game/VoxelStuff/Entities/Ball.h"

Voxel::VoxelScene::VoxelScene(CommonResources *resources) 
	: FullResourceHolder(resources)
	, m_GSpace(resources)
	, m_World(m_GSpace.FindShapeyRaw("")->AddChild<Voxel::VoxelWorld>(WorldStuff{ &m_Boi, &m_Boi, &m_Boi }))
	, m_Ass(m_GSpace.FindShapeyRaw("")->AddChild<Voxel::VoxelPlayer>(m_World.get(), VoxelPlayerStuff{ {0.f, 10.f, 0.f}, {0.f, 0.f, -1.f} }))
	, m_UI(resources)
{
	(void)m_GSpace.FindShapeyRaw("")->AddChild((G1::IShape *)new G1I::LightShape({ &m_GSpace, resources, "Rando Light" }, Light{ {10.f, 10.f, 0.f, 0.f}, {-0.7f, -0.7f, 0.f, 0.f}, {0.f, 0.f, 0.f, 0.f}, {0.f, 0.f, 0.f, 0.f}, {1.f, 1.f, 1.f}, 0.f, {1.f, 1.f, 1.f}, 35.f, 1, LIGHT_DIRECTION, {0.f, 0.f} }));

	(void)m_GSpace.FindShapeyRaw("")->AddChild(new G1I::ProfilerShape({ &m_GSpace, resources, "Profiler McGee" }, { 10.0, false }));

	(void)m_GSpace.FindShapeyRaw("")->AddChild(new G1I::GLRen2TestShape({ &m_GSpace, resources, "GLRen2 test" }));

	m_UI.AddChildTop(&m_Crosshair);

	m_Crosshair.Initialize();

	m_World->AddEntity(std::make_unique<Voxel::Entities::Ball>(m_World.get(), mResources, floaty3{ 0.f, 100.f, 0.f }));
}

Voxel::VoxelScene::~VoxelScene()
{
}

Debug::DebugReturn Voxel::VoxelScene::Initialize()
{
	m_World->Update(m_Ass->GetPosition());


	return true;
}

bool Voxel::VoxelScene::Receive(Events::IEvent *event)
{
	(void)event;
	return false;
}

void Voxel::VoxelScene::BeforeDraw()
{
	m_GSpace.BeforeDraw();
}

void Voxel::VoxelScene::Draw()
{
	CHECK_GL_ERR("Before Clearing OpenGL Buffers");
	glClearColor(0.01f, 0.01f, 0.01f, 1.f);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	CHECK_GL_ERR("Clearing OpenGL Buffers");
	m_GSpace.Draw();
	m_UI.Draw();
}

void Voxel::VoxelScene::AfterDraw()
{
	//m_World->Update(m_Ass->GetPosition());
	m_GSpace.AfterDraw();
}

Scene::IScene *Voxel::VoxelScene::Clone()
{
	return nullptr;
}

void Voxel::TheBoi::DisplaceWorld(floaty3 by)
{
	// TODO: this shit
}

Voxel::ChunkyBoi::RawChunkDataMap Voxel::TheBoi::LoadChunk(int64_t x, int64_t y, int64_t z)
{
	if (y == 0)
	{
		return
		{
			{ { 0, 0, 0}, { 1 } }, { { 0, 0, 1}, { 1 } }, { { 0, 0, 2}, { 1 } }, { { 0, 0, 3}, { 1 } }, { { 0, 0, 4}, { 1 } }, { { 0, 0, 5}, { 1 } }, { { 0, 0, 6}, { 1 } }, { { 0, 0, 7}, { 1 } },
			{ { 1, 0, 0}, { 1 } }, { { 1, 0, 1}, { 1 } }, { { 1, 0, 2}, { 1 } }, { { 1, 0, 3}, { 1 } }, { { 1, 0, 4}, { 1 } }, { { 1, 0, 5}, { 1 } }, { { 1, 0, 6}, { 1 } }, { { 1, 0, 7}, { 1 } },
			{ { 2, 0, 0}, { 1 } }, { { 2, 0, 1}, { 1 } }, { { 2, 0, 2}, { 1 } }, { { 2, 0, 3}, { 1 } }, { { 2, 0, 4}, { 1 } }, { { 2, 0, 5}, { 1 } }, { { 2, 0, 6}, { 1 } }, { { 2, 0, 7}, { 1 } },
			{ { 3, 0, 0}, { 1 } }, { { 3, 0, 1}, { 1 } }, { { 3, 0, 2}, { 1 } }, { { 3, 0, 3}, { 1 } }, { { 3, 0, 4}, { 1 } }, { { 3, 0, 5}, { 1 } }, { { 3, 0, 6}, { 1 } }, { { 3, 0, 7}, { 1 } },
			{ { 4, 0, 0}, { 1 } }, { { 4, 0, 1}, { 1 } }, { { 4, 0, 2}, { 1 } }, { { 4, 0, 3}, { 1 } }, { { 4, 0, 4}, { 1 } }, { { 4, 0, 5}, { 1 } }, { { 4, 0, 6}, { 1 } }, { { 4, 0, 7}, { 1 } },
			{ { 5, 0, 0}, { 1 } }, { { 5, 0, 1}, { 1 } }, { { 5, 0, 2}, { 1 } }, { { 5, 0, 3}, { 1 } }, { { 5, 0, 4}, { 1 } }, { { 5, 0, 5}, { 1 } }, { { 5, 0, 6}, { 1 } }, { { 5, 0, 7}, { 1 } },
			{ { 6, 0, 0}, { 1 } }, { { 6, 0, 1}, { 1 } }, { { 6, 0, 2}, { 1 } }, { { 6, 0, 3}, { 1 } }, { { 6, 0, 4}, { 1 } }, { { 6, 0, 5}, { 1 } }, { { 6, 0, 6}, { 1 } }, { { 6, 0, 7}, { 1 } },
			{ { 7, 0, 0}, { 1 } }, { { 7, 0, 1}, { 1 } }, { { 7, 0, 2}, { 1 } }, { { 7, 0, 3}, { 1 } }, { { 7, 0, 4}, { 1 } }, { { 7, 0, 5}, { 1 } }, { { 7, 0, 6}, { 1 } }, { { 7, 0, 7}, { 1 } },
		};
	}

	return {};
}

void Voxel::TheBoi::UnloadChunk(std::unique_ptr<ChunkyBoi> &&chunk)
{
	// TODO: this shit
}
