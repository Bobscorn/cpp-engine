#pragma once

#include "../Progress.h"
#include "../Stages/IntroStage.h"
#include "../Stages/Intro3DStage.h"
#include "../Stages/PrecipiceStages.h"

#include "Systems/Execution/Scene.h"
#include "Systems/Execution/ResourceHolder.h"
#include "Systems/Input/Importer.h"

#include "Structure/BasicShapes.h"

#include <memory>
#include <unordered_map>
#include <string>


namespace Scene
{
	struct DudStage : IStage
	{
		DudStage() : IStage(nullptr), SpokeDebugShit(false) {}

		void BeforeDraw() override {}
		void Draw() override {}
		void AfterDraw() override { if (!SpokeDebugShit) { DINFO("DudStage here, just telling you I'm being run when maybe you were wanting a real stage "); SpokeDebugShit = true; } }

		bool SpokeDebugShit = false;
	};

	struct StageScene : IScene, FullResourceHolder, IStageManager
	{
		StageScene(CommonResources *resources) : FullResourceHolder(resources), ActiveStage(std::make_unique<DudStage>()), m_ProfileBoi({nullptr, resources, "ass"}, {10.0, false})
		{
			Stages.try_emplace("Intro", std::make_unique<IntroStage::IntroCreator>());
			Stages.try_emplace("Prologue", std::make_unique<PrologueStage::PrologueCreator>());
			Stages.try_emplace("Precipice1", std::make_unique<Precipice::IntroStage::Creator>());
			Stages.try_emplace("Precipice2", std::make_unique<Precipice::SecondStage::Creator>());
			Stages.try_emplace("Precipice3", std::make_unique<Precipice::ThirdStage::Creator>());
			
			CheckSaveData();
		}

		std::unique_ptr<IStage> ActiveStage;
	protected:
		std::string ToSwitch;
	public:

		virtual Debug::DebugReturn Initialize() override;
		virtual bool Receive(Events::IEvent * event) override;
		virtual void BeforeDraw() override;
		virtual void Draw() override;
		virtual void AfterDraw() override;
		inline virtual IScene * Clone() override { return new StageScene(mResources); }

		// IStageManager
		virtual void ChangeStageTo(Stringy newstagename) override;
		virtual void SetBonusData(Stringy Key, Stringy Data) override;
		virtual bool BonusKeyExists(Stringy Key) override;
		virtual bool GetBonusKey(Stringy, Stringy &out) const override;
		virtual Stringy GetCurrentStage() const override;

		Debug::DebugReturn CheckSaveData();
		bool CheckForManualLoads();


		inline bool CanContinue() const { return ContinueData.CurrentStage.size(); }
		inline bool CanLoad() const { return Loads.size(); }
		inline std::vector<Stringy> GetLoadNames() const { std::vector<Stringy> names(Loads.size()); for (auto i = Loads.size(); i-- > 0; ) names[i] = Loads[i].CurrentStage; return names; }

		inline void ActivateContinueSave() { if (CanContinue()) { if (!PrepareData(ContinueData)) DWARNING("Something went wrong loading the continue save data"); } else DWARNING("Attempted Continuing when not possibles"); }

		bool PrepareData(ProgressData const &data);
		inline bool PrepareData(Stringy loadname)
		{
			for (auto& data : Loads)
				if (loadname == data.CurrentStage)
					return PrepareData(data);
			DWARNING("Attempt at preparing non-existant stage");
			return false;
		}

	protected:
		static bool IsValidSaveFile(Stringy path, Importing::XMLThing &out);
		static ProgressData ConvertXMLToProgress(Importing::XMLThing const &in);
		void Save();

		inline void NullifyActive()
		{
			ActiveStage = std::make_unique<DudStage>();
		}

		ProgressData CurrentData;

		static std::unordered_map<Stringy, std::unique_ptr<IStageCreator>> Stages;

		std::vector<ProgressData> Loads;
		ProgressData ContinueData;

		G1I::ProfilerShape m_ProfileBoi;
	};


}