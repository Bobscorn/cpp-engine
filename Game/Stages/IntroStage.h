#pragma once

#include "../Progress.h"

#include <Drawing/Effects/Flickerer.h>
#include <Systems/Execution/ResourceHolder.h>
#include <Systems/Timer/Timer.h>
#include <Systems/Events/EventsBase.h>
#include <Systems/Sequence/TimeSequence.h>
#include <Systems/Sequence/TerminalSequence.h>



struct FlickerSequence : Sequence::ITimedSequenceElement
{
	FlickerSequence(CommonResources *resources) : Flickerer(resources) { Flickerer.SetDoUnFlicker(); }
	FlickerSequence(CommonResources *resources, float count, float duration) : Flickerer(resources, duration, count) { Flickerer.SetDoUnFlicker(); }

	virtual double Apply(double localtime) override
	{
		return double(Flickerer.Apply(float(localtime)));
	}

protected:
	Flicker Flickerer;
};

struct TerminalSequence : Sequence::ITimedSequenceElement, virtual FullResourceHolder, TextBrushHolder, NormalFontHolder
{
	TerminalSequence(CommonResources *resources) : FullResourceHolder(resources), TextBrushHolder("Terminal"), NormalFontHolder("Terminal") { EnsureFormat(); EnsureBrush(); }
	TerminalSequence(CommonResources *resources, Stringy content) : FullResourceHolder(resources), TextBrushHolder("Terminal"), NormalFontHolder("Terminal"), Content(content) { EnsureFormat(); EnsureBrush(); }

	virtual double Apply(double localtime) override;	

	void EnsureFormat();
	void EnsureBrush();
	size_t GetIndex(double localtime) const;
	void DrawTheText(Stringy amount) const;
	inline void SetContent(Stringy content) { Content = content; }

protected:
	Stringy Content{ "Default Terminal Message" };

	double Duration{ 10.f };
};

struct FadeSequence : Sequence::ITimedSequenceElement, virtual FullResourceHolder
{
	FadeSequence(CommonResources *resources, double Duration = 1.f) : FullResourceHolder(resources), Duration(Duration) {}

	double Apply(double localtime) override;

protected:
	double Duration{ 1.f };
};

struct Terminalv2 : Sequence::TerminalSequencev2
{
	Terminalv2(CommonResources *resources) : FullResourceHolder(resources), TerminalSequencev2(resources)
	{
		using Sequence::NormalTextTerminalElement;
		using Sequence::LoadingTerminalElement;
		using Sequence::Loading2TerminalElement;
		using Sequence::PauseTerminalElement;
		this->Add<LoadingTerminalElement>(4.f);
		this->Add<NormalTextTerminalElement>("Encapsulated v2.bc52.87d\n");
		this->Add<NormalTextTerminalElement>("Booting........... ");
		this->Add<LoadingTerminalElement>(2.f);
		/*this->Add<NormalTextTerminalElement>("\n");
		this->Add<NormalTextTerminalElement>("Found 4 Module(s) for initialization...\n");
		this->Add<PauseTerminalElement>(1.f);
		this->Add<NormalTextTerminalElement>("Initializing Memory Module...");
		this->Add<Loading2TerminalElement>(3.f);
		this->Add<NormalTextTerminalElement>("\n");
		this->Add<NormalTextTerminalElement>("Initializing AI Module...");
		this->Add<Loading2TerminalElement>(3.f);
		this->Add<NormalTextTerminalElement>("\n");
		this->Add<NormalTextTerminalElement>("Initializing Visualizer Module...");
		this->Add<Loading2TerminalElement>(3.f);
		this->Add<NormalTextTerminalElement>("\n");
		this->Add<NormalTextTerminalElement>("Initializing X$% Module...");
		this->Add<Loading2TerminalElement>(1.f);
		this->Add<NormalTextTerminalElement>("error.*&$`1\n\n");
		this->Add<PauseTerminalElement>(1.f);
		this->Add<NormalTextTerminalElement>("All Modules successfully initialized, engaging user link...");
		this->Add<Loading2TerminalElement>(2.f);*/
	}
};

struct IntroStage : IStage, FullResourceHolder
{
	IntroStage(CommonResources *resources, IStageManager *man) : IStage(man), FullResourceHolder(resources) { Actions.Add<Terminalv2>(resources); Actions.Reset(); }

	struct IntroCreator : IStageCreator
	{
		inline virtual std::unique_ptr<IStage> Create(CommonResources * resources, IStageManager *man) override { return std::make_unique<IntroStage>(resources, man); }
	};

	// Inherited via IStage
	virtual void BeforeDraw() override;
	virtual void Draw() override;
	virtual void AfterDraw() override;

protected:

	Sequence::TimeSequence Actions;

	bool IsDone{ false };
};

