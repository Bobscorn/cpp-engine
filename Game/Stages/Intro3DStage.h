#pragma once

#include "Systems/Execution/ResourceHolder.h"
#include "Systems/Requests/Requestable.h"

#include "Structure/BasicGSpace.h"

#include "../Progress.h"

struct PrologueStage : IStage, FullResourceHolder, Requests::IRequestable
{
	PrologueStage(CommonResources *resources, IStageManager *man);

	struct PrologueCreator : IStageCreator
	{
		// Inherited via IStageCreator
		std::unique_ptr<IStage> Create(CommonResources * resources, IStageManager * man) override;
	};

	void BeforeDraw() override;
	void Draw() override;
	void AfterDraw() override;

	virtual Debug::DebugReturn Request(Requests::Request &request) override;
	virtual std::string GetName() const override;

protected:
	G1I::BasicGSpace Environment;
};

