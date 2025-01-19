#include "ParkourScene.h"

#include <Structure/BasicShapes.h>
#include <Structure/Shapes/SkyBoxShape.h>
#include <Structure/Shapes/SimpleRenderShape.h>
#include <Game/VoxelStuff/VoxelPlayerUI.h>
#include <Systems/Input/Input.h>
#include <Systems/Events/Converter.h>
#include <Systems/Time/Time.h>
#include <Systems/Execution/Engine.h>
#include <Drawing/GeometryStore.h>
#include <Drawing/VoxelStore.h>

#include <Helpers/StringHelper.h>

#include <vector>
#include <utility>

#include "ParkourLevels.h"
#include "ParkourBlocks.h"

Parkour::ParkourScene::ParkourScene(CommonResources *resources, int level)
	: FullResourceHolder(resources)
	, m_Level(level)
	, m_GSpace(resources)
	, m_WorldShape(m_GSpace.GetRootShape()->AddChild<Voxel::VoxelWorld>("Voxel World", Voxel::WorldStuff{&m_Loader, &m_Loader, &m_ChunkMemory, nullptr, 2, 1, 2, 1}))
	, m_PlayerShape(m_GSpace.GetRootShape()->AddChild<Voxel::VoxelPlayer>("Voxel Player", m_WorldShape.get(), Voxel::VoxelPlayerStuff()))
	, m_LevelShape(m_GSpace.GetRootShape()->AddChild<ParkourLevelShape>("ParkourLevel Shape", Parkour::Levels[level]))
	, m_GeneratorShape(m_GSpace.GetRootShape()->AddChild<ParkourGeneratorShape>("Parkour Generator", m_LevelShape, m_WorldShape, false))
	, m_TrackerShape(m_GSpace.GetRootShape()->AddChild<PlayerTrackerShape>("Parkour Player Tracker", PlayerTrackingData{ m_PlayerShape, m_WorldShape, m_LevelShape, 1.f, -20.f }))
	, m_ParkourEndShape(m_GSpace.GetRootShape()->AddChild<ParkourEndShape>("Parkour End Shape", m_LevelShape, m_WorldShape))
	//, m_TorchShape(m_GSpace.GetRootShape()->AddChild<ParkourTorchShape>("Player Torch Shape", m_PlayerShape))
	, m_UI(resources)
	, m_Crosshair()
	, m_ControlsImage(resources, "Controls Image", std::make_unique<Drawing::SDLFileImage>(resources, "Textures/keyboard_controls.png"), UI1I::UIPosition({ -1.f, -1.f }, { -1.f, -1.f }, { 208.f, 198.f }))
	, m_HUD(resources)
	, m_Menu(resources)
	, m_FinishMenu(resources)
{
	m_GSpace.GetRootShape()->AddChild<G1I::ProfilerShape>("Profiler McGee Shape", G1I::ProfilerThings{ 15.0, false });
	m_GSpace.GetRootShape()->AddChild<G1I::SkyBoxShape>("Skybox Shape", "skybox", ".jpg");
	m_GSpace.GetRootShape()->AddChild<ParkourTimeMeasuringShape>("Epic Time Measuring");

	{
		Light sunLight{};
		sunLight.Color = floaty3{ 1.f, 1.f, 1.f };
		sunLight.Intensity = 0.75f;
		sunLight.Attenuation = floaty3{ 2.f, 0.25f, 0.05f };
		sunLight.Enabled = true;
		sunLight.PositionWS = floaty4{ 0.f, 0.f, 0.f, 1.f };
		sunLight.PositionVS = floaty4{};
		sunLight.DirectionWS = floaty4{ floaty3::Normalized(floaty3{ -0.5f, -0.5f, -0.5f }), 1.f };
		sunLight.DirectionVS = floaty4{};
		sunLight.Range = 50.f;
		sunLight.Type = LIGHT_DIRECTION;
		sunLight.SpotlightAngle = 0;
		sunLight.ShadowIndex = 0;

		m_GSpace.GetRootShape()->AddChild<G1I::LightShape>("Sun Light", sunLight);

		sunLight.DirectionWS = floaty4{ floaty3::Normalized(floaty3{ 1.f, 0.f, 1.f }), 1.f };
		sunLight.Intensity = 0.35f;

		m_GSpace.GetRootShape()->AddChild<G1I::LightShape>("Backup Sun Light", sunLight);
	}

	auto& voxelStore = Voxel::VoxelStore::GetMutable();
	voxelStore.RegisterUpdateBlock("lamp-light", std::make_unique<ParkourLightBlock>(&m_GSpace, mResources, m_WorldShape.get(), nullptr, Voxel::ChunkBlockCoord{}));
	voxelStore.RegisterUpdateBlock("checkpoint", std::make_unique<ParkourCheckpointBlock>(&m_GSpace, mResources, m_WorldShape.get(), nullptr, Voxel::ChunkBlockCoord{}));

	m_UI.AddChildBottom(&m_Crosshair);
	m_UI.AddChildBottom(&m_ControlsImage);
	m_Menu.AddTo(m_UI);
	m_Menu.Disable();
	m_HUD.AddTo(m_UI);
	m_FinishMenu.AddTo(m_UI);
	m_FinishMenu.Disable();
}

Parkour::ParkourScene::~ParkourScene()
{
	if (m_Paused)
		Resume();
}

Debug::DebugReturn Parkour::ParkourScene::Initialize()
{
	m_WorldShape->Update(m_PlayerShape->GetPosition());
	mResources->InputAttachment->Add(m_PlayerShape.get());

	if (!this->Events::IEventListener::ListeningTo)
		mResources->Event->Add((Events::IEventListener*)this);

	if (!this->Requests::IRequestable::GetMaster())
		mResources->Request->Add(this);

	m_GeneratorShape->Generate();
	m_PlayerShape->SetPosition(m_WorldShape->GetPhysPosFromBlockCoord(m_LevelShape->GetLevelData().StartPos) + floaty3{ 0.f, 1.5f, 0.f });

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
			if (key_event->KeyCode == SDLK_r)
			{
				mResources->Request->Request(Requests::Request{ "ReloadMaterials" });
				DINFO("Reloading Materials");
			}
			else if (key_event->KeyCode == SDLK_COMMA)
			{
				DINFO("Sending record time request");
				mResources->Request->Request(Requests::Request{ "RecordTime" });
			}
			else if (key_event->KeyCode == SDLK_PERIOD)
			{
				DINFO("Reloading meshes...");
				mResources->Request->Request(Requests::Request{ "ReloadMeshes" });
			}
		}
	}
	return false;
}

void Parkour::ParkourScene::BeforeDraw()
{
	m_GSpace.BeforeDraw();

	m_HUD.Update();
}

void Parkour::ParkourScene::Draw()
{
	CHECK_GL_ERR("Before clearing");
	glClearColor(0.f, 0.f, 0.f, 1.f);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	CHECK_GL_ERR("After clearing");
	m_GSpace.Draw();
	m_FinishMenu.Update();
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
		if (!m_FinishMenu.IsEnabled())
			Resume();
		return true;
	}
	else if (req.Name == "Pause")
	{
		Pause();
		return true;
	}
	else if (req.Name == "WinRun")
	{
		m_FinishMenu.Enable();
		Pause();
		m_Menu.Disable();
		return true;
	}
	else if (req.Name == "RestartRun")
	{
		m_WorldShape->Reset();
		m_PlayerShape->SetPosition(floaty3{ 0.f, 2.f, 0.f });
		m_PlayerShape->ResetState();
		m_PlayerShape->SetLookUp(floaty3{ 0.f, 0.f, -1.f }, floaty3{ 0.f, 1.f, 0.f });
		m_FinishMenu.Disable();
		m_GeneratorShape->Generate();
		m_TrackerShape->Reset();
		Resume();
		return true;
	}
	else if (req.Name == "ReturnToMenu")
	{
		mResources->Engine->SwitchScene(std::make_unique<Parkour::ParkourStartingScene>(mResources, std::unique_ptr<Parkour::IParkourDifficultyScene>(new Parkour::ParkourScene(mResources, 0))));
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

void Parkour::ParkourScene::SetDifficulty(int dif)
{
	m_Level = dif;
	if (m_Level < 0 || m_Level >= Levels.size())
	{
		DERROR("Can not set to level of index '" + std::to_string(dif) + "' out of bounds!");
		return;
	}
	m_LevelShape->SetLevelData(Levels[m_Level]);
}

Parkour::ParkourStartingScene::ParkourStartingScene(CommonResources* resources, std::unique_ptr<IParkourDifficultyScene> nextScene)
	: FullResourceHolder(resources)
	, UIRoot(resources)
	, Faderer(resources, 1.f)
	, Title(resources, "Parkour!")
	, UIContainer(resources, "background.jpeg", UI1I::ButtonyContainer::CENTRE_ALIGN, UI1I::ButtonyContainer::CENTRE_ALIGN)
	, NewEasyGameButton(resources, "BEGIN EASY RUN", Requests::Request("BeginEasyRun"))
	, NewMediumGameButton(resources, "BEGIN MEDIUM RUN", Requests::Request("BeginMediumRun"))
	, NewHardGameButton(resources, "BEGIN HARD RUN", Requests::Request("BeginHardRun"))
	, ExitButton(resources, "EXIT", Requests::Request("ExitGame"))
	, NextScene(std::move(nextScene))
{
	mResources->UIConfig->RegisterNewBrush("Title", { 200, 80, 120 ,255 });

	mResources->UIConfig->RegisterNewTextFormat("Title", { "NotoSans-Regular.ttf", 24 });

	resources->Request->Add(this);
}

Debug::DebugReturn Parkour::ParkourStartingScene::Initialize()
{
	mResources->InputAttachment->Add(&UIRoot);
	UIRoot.AddChildrenTop({ &UIContainer, &Title });
	UIContainer.AddButtons({ &NewEasyGameButton, &NewMediumGameButton, &NewHardGameButton, &ExitButton });

	return true;
}

bool Parkour::ParkourStartingScene::Receive(Events::IEvent* event)
{
	(void)event;
	return false;
}

void Parkour::ParkourStartingScene::BeforeDraw()
{
	UIRoot.BeforeDraw();
}

void Parkour::ParkourStartingScene::Draw()
{
	CHECK_GL_ERR("Before clearing");
	glClearColor(0.f, 0.f, 0.f, 1.f);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	CHECK_GL_ERR("After clearing");
	UIRoot.Draw();
	if (DoNewGame)
	{
		if (Faderer.Fade())
			ActuallyDoNewGame();
	}
}

void Parkour::ParkourStartingScene::AfterDraw()
{
	UIRoot.AfterDraw();
}

Debug::DebugReturn Parkour::ParkourStartingScene::Request(Requests::Request& action)
{
	if (action.Name == "BeginEasyRun")
	{
		NextScene->SetDifficulty(0);
		NewGame();
		return true;
	}
	else if (action.Name == "BeginMediumRun")
	{
		NextScene->SetDifficulty(1);
		NewGame();
		return true;
	}
	else if (action.Name == "BeginHardRun")
	{
		NextScene->SetDifficulty(2);
		NewGame();
		return true;
	}
	else if (action.Name == "UI")
	{
		if (action.Params.size())
		{
			if (action.Params[0] == "ToggleDebugDrawing")
			{
				UIRoot.ToggleDebugDraw();
				return true;
			}
			else if (action.Params[0] == "ToggleMouseMovement")
			{
				UIRoot.ToggleMouseMovement();
				return true;
			}
			else if (action.Params[0] == "ToggleHover")
			{
				UIRoot.ToggleHover();
				return true;
			}
		}
	}

	return false;
}

void Parkour::ParkourStartingScene::NewGame()
{
	Faderer.Reset();
	Faderer.Start();
	DoNewGame = true;
}

void Parkour::ParkourStartingScene::ActuallyDoNewGame()
{
	mResources->Engine->SwitchScene(std::move(NextScene));
	DINFO("Wooo new game time");
}
