#include "StartingScene.h"

#include "Helpers/StringHelper.h"

#include "Systems/Execution/Engine.h"

Scene::StartingScene::StartingScene(CommonResources* resources, std::unique_ptr<IScene> nextScene)
	: FullResourceHolder(resources)
	, UIRoot(resources)
	, Faderer(resources, 1.f)
	, Title(resources, "Encapsulated")
	, UIContainer(resources, "background.jpeg", UI1I::ButtonyContainer::CENTRE_ALIGN, UI1I::ButtonyContainer::CENTRE_ALIGN)
	, NewGameButton(resources, "NEW GAME", Requests::Request("NewGame"))
	, ContinueButton(resources, "CONTINUE", Requests::Request("ContinueGame"))
	, LoadButton(resources, "LOAD", Requests::Request("LoadGame"))
	, OptionsButton(resources, "OPTIONS", Requests::Request("OpenOptions"))
	, ExitButton(resources, "EXIT", Requests::Request("ExitGame"))
	, NextScene(std::move(nextScene))
	//, Testo(resources)
{
	mResources->UIConfig->RegisterNewBrush("Title", { 200, 80, 120 ,255 });

	mResources->UIConfig->RegisterNewTextFormat("Title", { "NotoSans-Regular.ttf", 24 });

	resources->Request->Add(this);
}

Debug::DebugReturn Scene::StartingScene::Initialize()
{
	mResources->InputAttachment->Add(&UIRoot);
	UIRoot.AddChildrenTop({ &UIContainer, &Title });
	UIContainer.AddButtons({ &NewGameButton, &ContinueButton, &LoadButton, &OptionsButton, &ExitButton });
	//UIRoot.AddChildTop(&Testo);
	
	return true;
}

bool Scene::StartingScene::Receive(Events::IEvent * event)
{
	(void)event;
	return false;
}

void Scene::StartingScene::BeforeDraw()
{
	UIRoot.BeforeDraw();
}

void Scene::StartingScene::Draw()
{
	UIRoot.Draw();
	if (DoNewGame)
	{
		if (Faderer.Fade())
			ActuallyDoNewGame();
	}
}

void Scene::StartingScene::AfterDraw()
{
	UIRoot.AfterDraw();
}

Debug::DebugReturn Scene::StartingScene::Request(Requests::Request & action)
{
	if (action.Name == "NewGame")
	{
		NewGame();
		return true;
	}
	else if (action.Name == "ContinueGame")
	{
		return Continue();
	}
	else if (action.Name == "LoadGame")
	{
		if (action.Params.size())
		{
			double in{ 0.f };
			if (StringHelper::IfDouble(action.Params[0], &in))
			{
				return LoadGame((unsigned int)in);
			}
		}
	}
	else if (action.Name ==  "UI")
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

void Scene::StartingScene::NewGame()
{
	Faderer.Reset();
	Faderer.Start();
	DoNewGame = true;
}

void Scene::StartingScene::ActuallyDoNewGame()
{
	mResources->Engine->SwitchScene(std::move(NextScene));
	DINFO("Wooo new game time");
}

Debug::DebugReturn Scene::StartingScene::Continue()
{
	NewGame();
	return true;
}

Debug::DebugReturn Scene::StartingScene::LoadGame(unsigned int loadindex)
{

	(void)loadindex;
	return Debug::DebugReturn();
}
