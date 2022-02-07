#include "StageScene.h"

#ifdef __linux__
#include <experimental/filesystem>
using namespace std::experimental;
#else
#include <filesystem>
using namespace std;
#endif


std::unordered_map<Stringy, std::unique_ptr<IStageCreator>> Scene::StageScene::Stages;

Debug::DebugReturn Scene::StageScene::Initialize()
{
	if (ToSwitch.empty() || ToSwitch == "None")
		this->ActiveStage = Stages.at("Precipice1")->Create(mResources, this);

	return true;
}

bool Scene::StageScene::Receive(Events::IEvent * event)
{
	(void)event;
	return false;
}

void Scene::StageScene::BeforeDraw()
{
	ActiveStage->BeforeDraw();
	m_ProfileBoi.BeforeDraw();
}

void Scene::StageScene::Draw()
{
	ActiveStage->Draw();
}

void Scene::StageScene::AfterDraw()
{
	ActiveStage->AfterDraw();
	if (ToSwitch.size() && ToSwitch != "None")
	{
		auto it = Stages.find(ToSwitch);
		if (it != Stages.end())
		{
			ActiveStage.reset();
			ActiveStage = it->second->Create(mResources, this);
		}
		Save();
		ToSwitch.clear();
	}
	m_ProfileBoi.AfterDraw();
}

void Scene::StageScene::ChangeStageTo(Stringy newstagename)
{
	ToSwitch = newstagename;
}

void Scene::StageScene::SetBonusData(Stringy Key, Stringy Data)
{
	CurrentData.BonusData[Key] = Data;
}

bool Scene::StageScene::BonusKeyExists(Stringy Key)
{
	return CurrentData.BonusData.find(Key) != CurrentData.BonusData.end();
}

bool Scene::StageScene::GetBonusKey(Stringy key, Stringy &out) const
{
	auto it = CurrentData.BonusData.find(key);
	if (it == CurrentData.BonusData.end())
		return false;
	out = it->second;
	return true;
}

Stringy Scene::StageScene::GetCurrentStage() const
{
	return CurrentData.CurrentStage;
}

Debug::DebugReturn Scene::StageScene::CheckSaveData()
{
	DINFO("Checking for manual save data");
	CheckForManualLoads();

	DINFO("Importing Last Save Data");
	Importing::BasicXMLImporter Import;
	if (Import.Import(mResources->WorkingDirectory + "Data\\lastsave.xml"))
	{
		ContinueData = ConvertXMLToProgress(Import.GetOutput());
		return true;
	}
	else
	{
		DINFO("After failure of importing Data\\lastsave.xml Intro Stage will begin");
	}
	return false;
}

bool Scene::StageScene::CheckForManualLoads()
{
	std::vector<std::pair<filesystem::directory_entry, Importing::XMLThing>> Data;
	try
	{
		for (const auto &p : filesystem::directory_iterator(mResources->WorkingDirectory + "Data"))
		{
			if (filesystem::is_regular_file(p.status()))
			{
				Stringy path = p.path().string();
				Importing::XMLThing daaaat;
				if (IsValidSaveFile(path, daaaat))
					Data.push_back(std::make_pair(p, daaaat));
			}
		}
	}
	catch (const filesystem::filesystem_error &e)
	{
		DWARNING(e.what());
	}

	// Got all them valid files
	// Sort by date

	std::vector<Importing::XMLThing> SortedData;

	// Keep finding, adding and removing the newest files
	while (Data.size())
	{
		size_t index{ 0u };
		filesystem::file_time_type time;
		for (auto i = Data.size(); i-- > 0; )
		{
			try
			{
				if (filesystem::last_write_time(Data[i].first.path()) > time)
				{
					time = filesystem::last_write_time(Data[i].first.path());
					index = i;
				}
			}
			catch (...)
			{
				continue;
			}
		}

		SortedData.push_back(Data[index].second);
		Data.erase(Data.begin() + index);
	}

	Loads.reserve(SortedData.size());
	for (unsigned int i = 0; i < SortedData.size(); ++i)
	{
		Loads.emplace_back(ConvertXMLToProgress(SortedData[i]));
	}

	return true;
}

bool Scene::StageScene::PrepareData(ProgressData const & data)
{
	ToSwitch = data.CurrentStage;

	// To implement

	return true;
}

bool Scene::StageScene::IsValidSaveFile(Stringy path, Importing::XMLThing & out)
{
	if (path.substr(path.size() - (sizeof(SaveFileExtension) / sizeof(wchar_t)), (sizeof(SaveFileExtension) / sizeof(wchar_t))) == SaveFileExtension) // Check if its an xml file
	{
		// Has Extension

		// Cheap hard code here but I know its xml
		Importing::BasicXMLImporter imp(path);
		if (imp)
		{
			auto output = imp.GetOutput();
			auto stage = output.Find("currentstage", { Importing::QP("value", "Any") });
			if (stage)
			{
				auto it = Stages.find(stage->Properties["value"]);
				if (it != Stages.end())
				{
					out = output;
					return true;
				}
				else
					DWARNING("Found proper save file with invalid stage value of '" + stage->Properties["value"] + "' file name: '" + path + "'");
			}
			else
			{
				DWARNING("Possible save file found (has correct extension) without a 'currentstage' element named: '" + path + "'");
			}
		}
		else
			DWARNING("Failed to import file '" + path + "' when scanning for save files");
	}
	return false;
}

ProgressData Scene::StageScene::ConvertXMLToProgress(Importing::XMLThing const & in)
{
	ProgressData bob;
	auto stage = Importing::Find("currentstage", { Importing::QP("value", "Any") }, in.GetChildren());
	if (stage)
	{
		if (stage->Properties.find("value") != stage->Properties.end())
		{
			auto stagestage = stage->Properties.at("value");
			auto it = Stages.find(stagestage);
			if (it == Stages.end())
				return ProgressData();

			bob.CurrentStage = stagestage;
		}
		else
		{
			DERROR("Imported XML data contained currentstage tag *without* a value property");
			return ProgressData();
		}
	}
	else
	{
		DERROR("Imported XML data did not contain a currentstage tag");
		return ProgressData();
	}

	auto bonusdat = Importing::Find("bonusdata", {}, in.GetChildren());
	if (bonusdat)
	{
		for (auto & dat : bonusdat->GetChildren())
		{
			auto & data = dat.second;
			auto val = data->Properties.find("value");
			Stringy value = (val == data->Properties.end() ? "" : val->second);
			bob.BonusData.try_emplace(data->Name(), value);
		}
	}

	return bob;
}

void Scene::StageScene::Save()
{
	std::vector<std::pair<std::string, std::string>> bonusdat;
	for (auto &dat : CurrentData.BonusData)
	{
		bonusdat.emplace_back(dat.first, dat.second);
	}
	Importing::XMLExporter Bob(Importing::XMLExportData({
			Importing::ExportElement{
				{"currentstage"},
				{{"value", CurrentData.CurrentStage }},
				{}
			},
			Importing::ExportElement{
				{"bonusdata"},
				{bonusdat},
				{}
			}
		}));

	Bob.Export("Data/lastsave.xml");
}
