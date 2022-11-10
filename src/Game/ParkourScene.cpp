#include "ParkourScene.h"

#include <Structure/BasicShapes.h>
#include <Structure/Shapes/SkyBoxShape.h>
#include <Structure/Shapes/SimpleRenderShape.h>
#include <Game/VoxelStuff/VoxelPlayerUI.h>
#include <Systems/Input/Input.h>
#include <Systems/Events/Converter.h>
#include <Systems/Time/Time.h>
#include <Drawing/GeometryStore.h>
#include <Drawing/VoxelStore.h>

#include <vector>
#include <utility>

#include "ParkourLevels.h"
#include "ParkourBlocks.h"

Parkour::ParkourScene::ParkourScene(CommonResources *resources, int level)
	: FullResourceHolder(resources)
	, m_Level(level)
	, m_GSpace(resources)
	, m_WorldShape(m_GSpace.GetRootShape()->AddChild<Voxel::VoxelWorld>("Voxel World", Voxel::WorldStuff{&m_Loader, &m_Loader, nullptr, 1, 1, 1, 1}))
	, m_PlayerShape(m_GSpace.GetRootShape()->AddChild<Voxel::VoxelPlayer>("Voxel Player", m_WorldShape.get(), Voxel::VoxelPlayerStuff()))
	, m_LevelShape(m_GSpace.GetRootShape()->AddChild<ParkourLevelShape>("ParkourLevel Shape", Parkour::Levels[level]))
	, m_GeneratorShape(m_GSpace.GetRootShape()->AddChild<ParkourGeneratorShape>("Parkour Generator", m_LevelShape, m_WorldShape))
	, m_TrackerShape(m_GSpace.GetRootShape()->AddChild<PlayerTrackerShape>("Parkour Player Tracker", PlayerTrackingData{ m_PlayerShape, m_WorldShape, m_LevelShape, 1.f, -20.f }))
	, m_ParkourEndShape(m_GSpace.GetRootShape()->AddChild<ParkourEndShape>("Parkour End Shape", m_LevelShape, m_WorldShape))
	, m_UI(resources)
	, m_Crosshair()
	, m_Menu(resources)
{
	Light light{};
	light.Color = floaty3{ 1.f, 1.f, 1.f };
	light.Intensity = 5.f;
	light.Attenuation = floaty3{ 2.f, 0.25f, 0.05f };
	light.Enabled = true;
	light.PositionWS = floaty4{ 0.f, 3.f, 0.f, 1.f };
	light.PositionVS = floaty4{};
	light.DirectionWS = floaty4{ 0.f, -1.f, 0.f, 1.f };
	light.DirectionVS = floaty4{};
	light.Range = 50.f;
	light.Type = LIGHT_SPOT;
	light.SpotlightAngle = Math::DegToRadF * 30.f;
	light.Padding = 0.f;

	m_LightShape = m_GSpace.GetRootShape()->AddChild<G1I::LightShape>("Epic Light", light);
	m_GSpace.GetRootShape()->AddChild<G1I::ProfilerShape>("Profiler McGee Shape", G1I::ProfilerThings{ 15.0, false });
	m_GSpace.GetRootShape()->AddChild<G1I::SkyBoxShape>("Skybox Shape", "skybox", ".jpg");

	Voxel::VoxelStore::GetMutable().RegisterUpdateBlock("lamp-light", std::make_unique<ParkourLightBlock>(&m_GSpace, mResources, m_WorldShape.get(), nullptr, Voxel::ChunkBlockCoord{}));

	resources->InputAttachment->Add(m_PlayerShape.get());
	m_UI.AddChildBottom(&m_Crosshair);
	m_Menu.AddTo(m_UI);
	m_Menu.Disable();

	if (!this->Events::IEventListener::ListeningTo)
		resources->Event->Add((Events::IEventListener*)this);

	if (!this->Requests::IRequestable::GetMaster())
		resources->Request->Add(this);
}

Debug::DebugReturn Parkour::ParkourScene::Initialize()
{
	m_WorldShape->Update(m_PlayerShape->GetPosition());

	return true;
}

bool Parkour::ParkourScene::Receive(Events::IEvent* event)
{
	if (auto* key_event = Events::ConvertEvent<Event::KeyInput>(event))
	{
		if (key_event->State && key_event->KeyCode == SDLK_ESCAPE)
		{
			if (m_Paused)
				Resume();
			else
				Pause();
		}
		else if (key_event->State)
		{
			if (key_event->KeyCode == SDLK_EQUALS)
			{
				m_LightShape->GetLight()->Attenuation.y += 0.05f;
				DINFO("Increasing Attenuation by 0.05");
			}
			if (key_event->KeyCode == SDLK_MINUS)
			{
				m_LightShape->GetLight()->Attenuation.y -= 0.05f;
				DINFO("Decreasing Attenuation by 0.05");
			}
			if (key_event->KeyCode == SDLK_q)
			{
				m_LightShape->GetLight()->PositionWS = floaty4{ m_PlayerShape->GetCentre(), 1.f };
				DINFO("Setting light to player position (" + std::to_string(m_PlayerShape->GetCentre().x) + ", " + std::to_string(m_PlayerShape->GetCentre().y) + ", " + std::to_string(m_PlayerShape->GetCentre().z) + ")");
			}
			if (key_event->KeyCode == SDLK_r)
			{
				mResources->Request->Request(Requests::Request{ "ReloadMaterials" });
				DINFO("Reloading Materials");
			}
		}
	}
	return false;
}

void Parkour::ParkourScene::BeforeDraw()
{
	m_GSpace.BeforeDraw();
}

void Parkour::ParkourScene::Draw()
{
	CHECK_GL_ERR("Before clearing");
	glClearColor(0.f, 0.f, 0.f, 1.f);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	CHECK_GL_ERR("After clearing");
	m_GSpace.Draw();
	m_UI.Draw();
}

void Parkour::ParkourScene::AfterDraw()
{
	m_WorldShape->Update(m_PlayerShape->GetPosition());
	m_GSpace.AfterDraw();
}

std::unique_ptr<Scene::IScene> Parkour::ParkourScene::Clone()
{
	return std::make_unique<ParkourScene>(mResources, m_Level);
}

Debug::DebugReturn Parkour::ParkourScene::Request(Requests::Request& req)
{
	if (req.Name == "Resume")
	{
		Resume();
		return true;
	}
	else if (req.Name == "Pause")
	{
		Pause();
		return true;
	}
	return false;
}

Stringy Parkour::ParkourScene::GetName() const
{
	return "ParkourScene";
}

void Parkour::ParkourScene::Pause()
{
	if (m_Paused)
		return;

	m_Menu.Enable();
	mResources->InputAttachment->Add(&m_UI);
	// Other pause logic eventually
	mResources->Time->SetTimeScale(0.f);

	m_Paused = true;
}

void Parkour::ParkourScene::Resume()
{
	if (!m_Paused)
		return;

	m_Menu.Disable();
	mResources->InputAttachment->Remove(&m_UI);
	// Other logic eventually
	mResources->Time->SetTimeScale(1.f);

	m_Paused = false;
}
