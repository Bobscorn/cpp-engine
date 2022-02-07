#pragma once

#include "Systems/Execution/ResourceHolder.h"
#include "Systems/Requests/Requestable.h"

#include "Structure/BasicGSpace.h"

#include "../Progress.h"

namespace Precipice
{
	struct IntroStage : IStage, FullResourceHolder, Requests::IRequestable
	{
		IntroStage(CommonResources *resources, IStageManager *man);

		struct Creator : IStageCreator
		{
			std::unique_ptr<IStage> Create(CommonResources *resources, IStageManager *man) override;
		};

		void BeforeDraw() override;
		void Draw() override;
		void AfterDraw() override;

		virtual Debug::DebugReturn Request(Requests::Request &request) override;
		virtual std::string GetName() const override;

	protected:
		G1I::BasicGSpace m_Environment;

	};

	struct SecondStage : IStage, FullResourceHolder, Requests::IRequestable
	{
		SecondStage(CommonResources *resources, IStageManager *man);

		struct Creator : IStageCreator
		{
			std::unique_ptr<IStage> Create(CommonResources *resources, IStageManager *man) override;
		};

		void BeforeDraw() override;
		void Draw() override;
		void AfterDraw() override;

		virtual Debug::DebugReturn Request(Requests::Request &request) override;
		virtual std::string GetName() const override;

	protected:
		G1I::BasicGSpace m_Environment;

	};

	struct ThirdStage : IStage, FullResourceHolder, Requests::IRequestable
	{
		ThirdStage(CommonResources *resources, IStageManager *man);

		struct Creator : IStageCreator
		{
			std::unique_ptr<IStage> Create(CommonResources *resources, IStageManager *man) override;
		};

		void BeforeDraw() override;
		void Draw() override;
		void AfterDraw() override;

		virtual Debug::DebugReturn Request(Requests::Request &request) override;
		virtual std::string GetName() const override;

	protected:
		G1I::BasicGSpace m_Environment;

	};
}