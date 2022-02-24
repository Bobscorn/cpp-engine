#pragma once

#include "../UI/GameUI.h"
#include "../UI/OptionsUI.h"

#include <Systems/Execution/Scene.h>
#include <Systems/Requests/Requester.h>
#include <Systems/Execution/ResourceHolder.h>
#include <Systems/Input/Importer.h>

#include <Drawing/Effects/Fade.h>

#include "StageScene.h"

namespace Scene
{
	struct StartingScene : IScene, FullResourceHolder, Requests::IRequestable
	{
		StartingScene(CommonResources *resources);

		// Inherited via IScene
		virtual Debug::DebugReturn Initialize() override;
		virtual bool Receive(Events::IEvent * event) override;
		virtual void BeforeDraw() override;
		virtual void Draw() override;
		virtual void AfterDraw() override;

		inline virtual IScene * Clone() override { return new StartingScene(mResources); }

		virtual Debug::DebugReturn Request(Requests::Request &action) override;
		inline virtual Stringy GetName() const override { return "StartingScene instance"; }

	protected:
		void NewGame();
		void ActuallyDoNewGame();
		[[nodiscard]] Debug::DebugReturn Continue();
		[[nodiscard]] Debug::DebugReturn LoadGame(unsigned int loadindex);

		UI1::RootElement UIRoot;
		UI1I::TitleText Title;
		UI1I::ButtonyContainer UIContainer;
		UI1I::NormalButtony NewGameButton;
		UI1I::NormalButtony ContinueButton;
		UI1I::NormalButtony LoadButton;
		UI1I::SmallButtony OptionsButton;
		UI1I::SmallButtony ExitButton;
		//Options::DefaultOptionMenu Testo;

		std::unique_ptr<StageScene> Stageboi;
		SolidFader Faderer;
		bool DoNewGame{ false };
	};
}