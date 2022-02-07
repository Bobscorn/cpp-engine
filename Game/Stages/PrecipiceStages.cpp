#include "PrecipiceStages.h"

#include "Structure/PerviousShapes.h"

namespace Precipice
{
	IntroStage::IntroStage(CommonResources *resources, IStageManager *man) 
		: FullResourceHolder(resources)
		, IStage(man)
		, m_Environment(resources)
	{
		auto *root = m_Environment.FindShapeyRaw("");

		if (root)
		{
			//root->AddChild<Perviousity::Player::PerviousPlayer>(Perviousity::Player::PlayerStuff{ floaty3{0.f, 1.f, 0.f} });
			Damn::Scene China = Damn::Scene{ "cubeprecipicetutorial.dae", { PullOptions::Put_up_with_it } };
			Damn::ImportSceneToShape(Damn::ImportOptions{ Damn::GetLevel1ImportOptions() }, China, root);

			auto* player = m_Environment.FindShapeyRaw("Puzzle Controller");
			if (!player)
			{
				DWARNING("No player supplied spawning one at (0, 1, 0)");
				root->AddChild(new Perviousity::Player::PerviousPlayer(root->GetContainer(), root->GetResources(), Perviousity::Player::PlayerStuff{ floaty3{0.f, 1.f, 0.f} }));
			}

			auto *control = m_Environment.FindShapeyRaw("Group Control 1");
			if (control)
			{
				auto *group = dynamic_cast<Perviousity::Shapes::GroupController *>(control);
				if (group)
				{
					group->SetDisableAbility(false);
				}
			}
		}

		mResources->Request->Add(this);
	}

	void IntroStage::BeforeDraw()
	{
		m_Environment.BeforeDraw();
	}

	void IntroStage::Draw()
	{
		CHECK_GL_ERR("Before Clearing OpenGL Buffers");
		glClearColor(0.01f, 0.01f, 0.01f, 1.f);
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
		CHECK_GL_ERR("Clearing OpenGL Buffers");
		m_Environment.Draw();
	}

	void IntroStage::AfterDraw()
	{
		m_Environment.AfterDraw();
	}

	Debug::DebugReturn IntroStage::Request(Requests::Request &request)
	{
		if (request.Name == "EndLevel")
		{
			Manager->ChangeStageTo("Precipice2");
			return true;
		}
		return false;
	}

	std::string IntroStage::GetName() const
	{
		return "Precipice Intro Stage";
	}

	std::unique_ptr<IStage> IntroStage::Creator::Create(CommonResources *resources, IStageManager *man)
	{
		return std::make_unique<Precipice::IntroStage>(resources, man);
	}

	SecondStage::SecondStage(CommonResources *resources, IStageManager *man)
		: FullResourceHolder(resources)
		, IStage(man)
		, m_Environment(resources)
	{
		auto *root = m_Environment.FindShapeyRaw("");

		if (root)
		{
			//root->AddChild<Perviousity::Player::PerviousPlayer>(Perviousity::Player::PlayerStuff{ floaty3{0.f, 1.f, 0.f} });
			Damn::Scene China = Damn::Scene{ "cubeprecipice.dae", { PullOptions::Put_up_with_it } };
			Damn::ImportSceneToShape(Damn::ImportOptions{ Damn::GetLevel1ImportOptions() }, China, root);

			auto* player = m_Environment.FindShapeyRaw("Puzzle Controller");
			if (!player)
				root->AddChild(new Perviousity::Player::PerviousPlayer(root->GetContainer(), root->GetResources(), Perviousity::Player::PlayerStuff{ floaty3{0.f, 1.f, 0.f} }));
			
			auto *control = m_Environment.FindShapeyRaw("Group Control 1");
			if (control)
			{
				auto *group = dynamic_cast<Perviousity::Shapes::GroupController *>(control);
				if (group)
				{
					group->SetDisableAbility(false);
				}
				else
				{
					DINFO("Could not find Group Control 1");
				}
			}
			else
			{
				DINFO("Could not find Group Control 1");
			}
		}

		mResources->Request->Add(this);
	}

	void SecondStage::BeforeDraw()
	{
		m_Environment.BeforeDraw();
	}

	void SecondStage::Draw()
	{
		CHECK_GL_ERR("Before Clearing OpenGL Buffers");
		glClearColor(0.01f, 0.01f, 0.01f, 1.f);
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
		CHECK_GL_ERR("Clearing OpenGL Buffers");
		m_Environment.Draw();
	}

	void SecondStage::AfterDraw()
	{
		m_Environment.AfterDraw();
	}

	Debug::DebugReturn SecondStage::Request(Requests::Request &request)
	{
		if (request.Name == "EndLevel")
		{
			Manager->ChangeStageTo("Precipice3");
			return true;
		}
		return false;
	}

	std::string SecondStage::GetName() const
	{
		return "Precipice Second Stage";
	}

	std::unique_ptr<IStage> SecondStage::Creator::Create(CommonResources *resources, IStageManager *man)
	{
		return std::make_unique<Precipice::SecondStage>(resources, man);
	}


	ThirdStage::ThirdStage(CommonResources *resources, IStageManager *man)
		: FullResourceHolder(resources)
		, IStage(man)
		, m_Environment(resources)
	{
		auto *root = m_Environment.FindShapeyRaw("");

		if (root)
		{
			//root->AddChild<Perviousity::Player::PerviousPlayer>(Perviousity::Player::PlayerStuff{ floaty3{0.f, 1.f, 0.f} });
			Damn::Scene China = Damn::Scene{ "precipice3.dae", { PullOptions::Put_up_with_it } };
			Damn::ImportSceneToShape(Damn::ImportOptions{ Damn::GetLevel1ImportOptions() }, China, root);

			auto *player = m_Environment.FindShapeyRaw("Puzzle Controller");
			if (!player)
				root->AddChild(new Perviousity::Player::PerviousPlayer(root->GetContainer(), root->GetResources(), Perviousity::Player::PlayerStuff{ floaty3{0.f, 1.f, 0.f} }));
		}

		mResources->Request->Add(this);
	}
	void ThirdStage::BeforeDraw()
	{
		m_Environment.BeforeDraw();
	}
	void ThirdStage::Draw()
	{
		CHECK_GL_ERR("Before Clearing OpenGL Buffers");
		glClearColor(0.01f, 0.01f, 0.01f, 1.f);
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
		CHECK_GL_ERR("Clearing OpenGL Buffers");
		m_Environment.Draw();
	}
	void ThirdStage::AfterDraw()
	{
		m_Environment.AfterDraw();
	}
	Debug::DebugReturn ThirdStage::Request(Requests::Request &request)
	{
		if (request.Name == "EndLevel")
		{
			DINFO("Woo you won");
			return true;
		}
		return false;
	}
	std::string ThirdStage::GetName() const
	{
		return "Precipice Level 3";
	}
	std::unique_ptr<IStage> ThirdStage::Creator::Create(CommonResources *resources, IStageManager *man)
	{
		return std::make_unique<ThirdStage>(resources, man);
	}
}