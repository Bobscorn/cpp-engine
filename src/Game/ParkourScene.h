#pragma once

#include <Helpers/PointerHelper.h>

#include <Systems/Execution/Scene.h>
#include <Systems/Requests/Requestable.h>
#include <Structure/BasicGSpace.h>
#include <Game/Scene/VoxelScene.h>
#include <Game/VoxelStuff/VoxelWorld.h>
#include <Game/VoxelStuff/VoxelPlayer.h>
#include <Game/VoxelStuff/VoxelPlayerUI.h>

#include <Drawing/Effects/Fade.h>

#include "ParkourShapes.h"
#include "ParkourTorchShape.h"
#include "ParkourUI.h"

#include <array>
#include <memory>

namespace G1I
{
	struct LightShape;
}

namespace Parkour
{
	class IParkourDifficultyScene : public virtual Scene::IScene
	{
	public:
		virtual ~IParkourDifficultyScene() {}

		// 0 - Easy, 1 - Medium, 2 - Hard
		virtual void SetDifficulty(int dif) = 0;
	};

	class ParkourScene : public virtual Scene::IScene, public FullResourceHolder, public Requests::IRequestable, public Events::IEventListenerT<Events::KeyEvent>, public IParkourDifficultyScene
	{
	public:
		ParkourScene(CommonResources *resources, int level);
		
		// Inherited via IScene
		virtual Debug::DebugReturn Initialize() override;

		virtual bool Receive(Events::IEvent* event) override;

		virtual void BeforeDraw() override;

		virtual void Draw() override;

		virtual void AfterDraw() override;

		virtual std::unique_ptr<IScene> Clone() override;

		virtual Debug::DebugReturn Request(Requests::Request& req) override;
		virtual Stringy GetName() const override;

		void Pause();
		void Resume();

		virtual void SetDifficulty(int difficulty) override;

	protected:
		int m_Level = 0;

		G1I::BasicGSpace m_GSpace;

		Voxel::DefaultWorldLoader m_Loader;
		Pointer::f_ptr<Voxel::VoxelWorld> m_WorldShape;

		Pointer::f_ptr<Voxel::VoxelPlayer> m_PlayerShape;
		Pointer::f_ptr<ParkourLevelShape> m_LevelShape;
		Pointer::f_ptr<ParkourGeneratorShape> m_GeneratorShape;
		Pointer::f_ptr<PlayerTrackerShape> m_TrackerShape;
		Pointer::f_ptr<ParkourEndShape> m_ParkourEndShape;
		Pointer::f_ptr<ParkourTorchShape> m_TorchShape;

		UI1::RootElement m_UI;
		Voxel::Crosshair m_Crosshair;
		ParkourInGameMenu m_Menu;
		ParkourInGameHUD m_HUD;
		bool m_Paused = false;
	};

	class ParkourStartingScene : public Scene::IScene, FullResourceHolder, Requests::IRequestable
	{
	public:
		ParkourStartingScene(CommonResources* resources, std::unique_ptr<IParkourDifficultyScene> nextScene);

		// Inherited via IScene
		virtual Debug::DebugReturn Initialize() override;
		virtual bool Receive(Events::IEvent* event) override;
		virtual void BeforeDraw() override;
		virtual void Draw() override;
		virtual void AfterDraw() override;

		inline virtual std::unique_ptr<IScene> Clone() override { return std::make_unique<ParkourStartingScene>(mResources, std::unique_ptr<IParkourDifficultyScene>(dynamic_cast<IParkourDifficultyScene*>((NextScene ? NextScene->Clone().release() : nullptr)))); }

		virtual Debug::DebugReturn Request(Requests::Request& action) override;
		inline virtual Stringy GetName() const override { return "ParkourStartingScene instance"; }

	protected:
		void NewGame();
		void ActuallyDoNewGame();

		UI1::RootElement UIRoot;
		UI1I::TitleText Title;
		UI1I::ButtonyContainer UIContainer;
		UI1I::NormalButtony NewEasyGameButton;
		UI1I::NormalButtony NewMediumGameButton;
		UI1I::NormalButtony NewHardGameButton;
		UI1I::SmallButtony ExitButton;

		std::unique_ptr<IParkourDifficultyScene> NextScene;
		SolidFader Faderer;
		bool DoNewGame{ false };
	};
}
