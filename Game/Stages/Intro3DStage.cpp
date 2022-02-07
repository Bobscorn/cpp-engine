#include "Intro3DStage.h"

#include "Helpers/TransferHelper.h"
#include "Helpers/GSpaceHelper.h"

#include "Structure/BasicShapes.h"
#include "Structure/PuzzleShapes.h"
#include "Structure/PerviousShapes.h"
#include "Structure/PerviousShit.h"
#include "Structure/TestShapes.h"

#include "Config/Config.h"

#include "Imported/Level1.h"


PrologueStage::PrologueStage(CommonResources * resources, IStageManager * man) : IStage(man), FullResourceHolder(resources), Environment(resources)
{
	auto *root = Environment.FindShapeyRaw("");
	
	if (root)
	{
		mResources->Config->SafeSet(Config::Key{ "OutOfBoundsLevel" }, -50.f);

		//root->AddChild<Perviousity::Player::PerviousPlayer>(Perviousity::Player::PlayerStuff{ floaty3{0.f, 1.f, 0.f} });
		Damn::Scene China = Damn::Scene{ "cubeprecipice.dae", { PullOptions::Put_up_with_it } };
		Damn::ImportSceneToShape(Damn::ImportOptions{ Damn::GetLevel1ImportOptions() }, China, root);
	}

	mResources->Request->Add(this);
}

void PrologueStage::BeforeDraw()
{
	Environment.BeforeDraw();
}

void PrologueStage::Draw()
{
	CHECK_GL_ERR("Before Clearing OpenGL Buffers");
	glClearColor(0.1f, 0.3f, 0.5f, 1.f);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	CHECK_GL_ERR("Clearing OpenGL Buffers");
	Environment.Draw();
}

void PrologueStage::AfterDraw()
{
	Environment.AfterDraw();
}

Debug::DebugReturn PrologueStage::Request(Requests::Request &request)
{
	if (request.Name == "EndLevel")
	{
		Manager->ChangeStageTo("Level2");
		return true;
	}
	return false;
}

std::string PrologueStage::GetName() const
{
	return "Level1Stage Object";
}

std::unique_ptr<IStage> PrologueStage::PrologueCreator::Create(CommonResources * resources, IStageManager * man)
{
	return std::unique_ptr<IStage>(new PrologueStage(resources, man));
}