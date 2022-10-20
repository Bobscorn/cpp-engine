#pragma once


#include <memory>
#include <unordered_map>
#include <string>

constexpr const char *SaveFileExtension = ".xml";

struct CommonResources;

struct ProgressData
{
	std::string CurrentStage;
	std::unordered_map<std::string, std::string> BonusData;
};

struct IStageManager
{
	virtual void ChangeStageTo(std::string newstagename) = 0;
	virtual void SetBonusData(std::string Key, std::string Data) = 0;

	virtual bool BonusKeyExists(std::string Key) = 0;
	virtual bool GetBonusKey(std::string key, std::string &out) const = 0;
	virtual std::string GetCurrentStage() const = 0;
	virtual void Save() = 0;
};

struct IStage
{
	IStage(IStageManager *man) : Manager(man) {}
	virtual ~IStage() {}

	virtual void BeforeDraw() = 0;
	virtual void Draw() = 0;
	virtual void AfterDraw() = 0;

protected:
	IStageManager *Manager{ nullptr };
};

struct IStageCreator
{
	virtual ~IStageCreator() {}

	virtual std::unique_ptr<IStage> Create(CommonResources *resources, IStageManager *man) = 0;
};

struct DudStage : IStage
{
	DudStage() : IStage(nullptr) {}
	
	inline void BeforeDraw() override {}
	inline void Draw() override {}
	inline void AfterDraw() override {}
};