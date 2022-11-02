#include "VoxelScene.h"

#include <Structure/BasicShapes.h>

#include "Game/VoxelStuff/Entities/Ball.h"
#include "Drawing/VoxelStore.h"

#include "Structure/Shapes/SkyBoxShape.h"

Voxel::VoxelScene::VoxelScene(CommonResources *resources) 
	: FullResourceHolder(resources)
	, m_GSpace(resources)
	, m_World(m_GSpace.FindShapeyRaw("")->AddChild<Voxel::VoxelWorld>("Voxel World", WorldStuff{&m_Boi, &m_Boi, &m_Boi, 2, 1, 2}))
	, m_Player(m_GSpace.FindShapeyRaw("")->AddChild<Voxel::VoxelPlayer>("Voxel Player", m_World.get(), VoxelPlayerStuff{{0.f, 10.f, 0.f}, {0.f, 0.f, -1.f}}))
	, m_UI(resources)
{
	(void)m_GSpace.FindShapeyRaw("")->AddChild(new G1I::LightShape({ &m_GSpace, resources, "Rando Light" }, Light{ {10.f, 10.f, 0.f, 0.f}, {-0.7f, -0.7f, 0.f, 0.f}, {0.f, 0.f, 0.f, 0.f}, {0.f, 0.f, 0.f, 0.f}, {1.f, 1.f, 1.f}, 1.f, {1.f, 0.f, 0.f}, 0.f, 35.f, 1, LIGHT_DIRECTION, 0.f }));
	(void)m_GSpace.FindShapeyRaw("")->AddChild((G1::IShape *)new G1I::LightShape({ &m_GSpace, resources, "Rando Point Light" }, Light{ {0.f, 0.5f, 0.f, 0.f}, {0.f, 0.f, 0.f, 0.f}, {0.f, 0.f, 0.f, 0.f}, {0.f, 0.f, 0.f, 0.f}, {1.f, 1.f, 1.f}, 1.f, {0.1f, 0.5f, 0.f}, 0.f, 35.f, 1, LIGHT_POINT, 0.f }));

	(void)m_GSpace.FindShapeyRaw("")->AddChild(new G1I::ProfilerShape({ &m_GSpace, resources, "Profiler McGee" }, { 10.0, false }));

	(void)m_GSpace.FindShapeyRaw("")->AddChild(new G1I::GLRen2TestShape({ &m_GSpace, resources, "GLRen2 test" }));
	(void)m_GSpace.FindShapeyRaw("")->AddChild(new G1I::SkyBoxShape({ &m_GSpace, resources, "Skybox Shape" }, "Textures/skybox", ".bmp"));

	m_UI.AddChildTop(&m_Crosshair);

	m_Crosshair.Initialize();

	m_World->AddEntity(std::make_unique<Voxel::Entities::Ball>(m_World.get(), mResources, floaty3{ 0.f, 100.f, 0.f }));
}

Voxel::VoxelScene::~VoxelScene()
{
}

Debug::DebugReturn Voxel::VoxelScene::Initialize()
{
	m_World->Update(m_Player->GetPosition());


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
	m_World->Update(m_Player->GetPosition());
	m_GSpace.AfterDraw();
}

std::unique_ptr<Scene::IScene> Voxel::VoxelScene::Clone()
{
	return std::make_unique<VoxelScene>(mResources);
}

void Voxel::DefaultWorldLoader::DisplaceWorld(floaty3 by)
{
	(void)by;
	// TODO: this shit
}

Voxel::RawChunkDataMap Voxel::DefaultWorldLoader::LoadChunk(int64_t x, int64_t y, int64_t z)
{
	(void)x;
	(void)z;
	if (y == 0)
	{
		Voxel::RawChunkDataMap map;
		for (unsigned int x_i = 0; x_i < Chunk_Size; ++x_i)
		{
			for (unsigned int z_i = 0; z_i < Chunk_Size; ++z_i)
			{
				map[{x_i, 0, z_i}] = VoxelStore::Instance().GetDescOrEmpty("wood")->BlockData;
			}
		}
		return map;
	}

	return {};
}

void Voxel::DefaultWorldLoader::UnloadChunk(std::unique_ptr<ChunkyBoi> chunk)
{
	(void)chunk;
	// TODO: this shit
}
