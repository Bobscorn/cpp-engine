#pragma once

#include <Helpers/PointerHelper.h>

#include <Systems/Execution/Scene.h>
#include <Systems/Requests/Requestable.h>
#include <Structure/BasicGSpace.h>
#include <Game/Scene/VoxelScene.h>
#include <Game/VoxelStuff/VoxelWorld.h>
#include <Game/VoxelStuff/VoxelPlayer.h>
#include <Game/VoxelStuff/VoxelPlayerUI.h>

#include "ParkourShapes.h"
#include "ParkourUI.h"

#include <array>
#include <memory>

namespace G1I
{
	struct LightShape;
}

namespace Parkour
{
	class ParkourScene : public Scene::IScene, public FullResourceHolder, public Requests::IRequestable, public Events::IEventListenerT<Events::KeyEvent>
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
		Pointer::f_ptr<G1I::LightShape> m_LightShape;

		UI1::RootElement m_UI;
		Voxel::Crosshair m_Crosshair;
		ParkourInGameMenu m_Menu;
		bool m_Paused = false;
	};
}
