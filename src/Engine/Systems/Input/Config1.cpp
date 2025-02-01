#include "Config1.h"
#include "WindowConfig.h"

#include "Helpers/DebugHelper.h"
#include "Helpers/StringHelper.h"
#include "Helpers/VectorHelper.h"
#include "Helpers/TransferHelper.h"

#include "Systems/Events/Events.h"
#include "Systems/Requests/Requester.h"
#include "Systems/Interaction/Interactor.h"
#include "Importer.h"

#include "Drawing/Graphics1.h"

#include "Systems/Thing.h"

const Stringy Config1::WindowConfig::DefaultWindowConfigFile	= ConfigPrefix + "windowconfig.xml";
const Stringy Config1::UIConfig::DefaultUIConfigFile			= ConfigPrefix + "uiconfig.xml";

void Config1::WindowConfig::Reset(Stringy Filename)
{
	Importing::BasicXMLImporter ass(Filename);

	if (!ass)
	{
		DWARNING("failed to import '" + Filename + "'");
		return;
	}

	Reset(ass.GetOutput().GetChildren());
}

void Config1::WindowConfig::Reset(std::unordered_map<Importing::XMLKey, Importing::bob<Importing::XMLElement>> &xmlbits)
{
	using Importing::Find;
	using Importing::QP;
	auto posel = Find("position", { QP("x", "Any"), QP("y", "Any") }, xmlbits);
	if (posel)
	{
		DINFO("found old window position");
		if (posel->Properties["x"].size())
			X = std::stoi(posel->Properties["x"]);

		if (posel->Properties["y"].size())
			Y = std::stoi(posel->Properties["y"]);
	}

	auto dimel = Find("dimensions", { QP("width", "Any"), QP("height", "Any") }, xmlbits);
	if (dimel)
	{
		DINFO("found old window dimensions");
		if (dimel->Properties["width"].size())
			Width = std::stoi(dimel->Properties["width"]);

		if (dimel->Properties["height"].size())
			Height = std::stoi(dimel->Properties["height"]);
	}

	auto screenel = Find("screen", {}, xmlbits);
	if (screenel)
	{
		DINFO("found old screen state");
		auto fullscreenel = Find("fullscreen", { QP("value", "Any") }, screenel->GetChildren());
		if (fullscreenel)
			FullScreen = StringHelper::StoB(fullscreenel->Properties["value"]);
		auto maximel = Find("maximized", { QP("value", "Any") }, screenel->GetChildren());
		if (maximel)
			Maximized = StringHelper::StoB(maximel->Properties["value"]);
	}

	if (Width <= 0 || Height <= 0) // Bad window file
	{
		Width = 800;
		Height = 600;
		X = 200;
		Y = 200;
		FullScreen = false;
		Maximized = false;
	}
}

void Config1::WindowConfig::WriteToFile(Stringy workdir, Stringy FileName)
{
	Importing::XMLExporter exp(Importing::XMLExportData(
		{
			{
				Stringy("window"),
				{},
				{
					{
						Stringy("position"),
						{ {Stringy("x"), std::to_string(X)}, {Stringy("y"), std::to_string(Y)} },
						{}
					},
					{
						Stringy("dimensions"),
						{ {Stringy("width"),std::to_string(Width)}, {Stringy("height"), std::to_string(Height)} },
						{}
					},
					{
						Stringy("screen"),
						{},
						{
							{
								Stringy("fullscreen"),
								{ {Stringy("value"), std::to_string(FullScreen),} },
								{}
							},
							{
								Stringy("maximized"),
								{ {Stringy("value"), std::to_string(Maximized) } },
								{}
							}
						}
					}
				}
			}
		}
	));

	exp.Export(workdir + FileName);
}

Config1::UIConfig::UIConfig(Events::EventManager *e, float dpi, Stringy Filename)
{
	if (e)
	{
		if (!this->IEventListener::ListeningTo)
			e->Add(this);
		
		FinalBigSize = BigFontSize * FontScale * dpi;
		FinalNormalSize = NormalFontSize * FontScale * dpi;
		FinalSmallSize = SmallFontSize * FontScale * dpi;
		FinalTooltipSize = TooltipFontSize * FontScale * dpi;

		RegisterNewTextFormat("Small", { "Avara.ttf", 12 });
		RegisterNewTextFormat("Normal", { "Avara.ttf", 26 });
		RegisterNewTextFormat("Big", { "Avara.ttf", 34 });
		RegisterNewTextFormat("Title", { "Avara.ttf", 48 });

		RegisterNewBrush("Text", { 200, 200, 200, 255 });
		RegisterNewBrush("HoverText", { 220, 220, 220, 255 });
		RegisterNewBrush("MouseDownText", { 100, 100, 100, 255 });
		RegisterNewBrush("DebugLine", { 200, 200, 100, 255 });
		RegisterNewBrush("Title", { 200, 80, 120, 255 });
	}
	else
		DERROR("Given null eventmanager");
}

void Config1::UIConfig::Reset(Stringy Filename)
{
	Stringy Fullpath = mResources->WorkingDirectory + Filename;
	Importing::BasicXMLImporter ass(Fullpath);
	DINFO("importing '" + Fullpath + "' as UIConfig");

	if (!ass)
	{
		DWARNING("failed to import '" + Fullpath + "'");
		return;
	}

	Reset(ass.GetOutput().GetChildren());
}

void Config1::UIConfig::Reset(std::unordered_map<Importing::XMLKey, Importing::bob<Importing::XMLElement>> &xmlbits)
{
	(void)xmlbits;
	// Code was deleted
}

bool Config1::UIConfig::Receive(Events::IEvent *event)
{
	if (auto *sass = Events::ConvertEvent<Event::ResizePreEvent>(event))
	{
		DoFontSizes();
		ResizeFonts(1.f);
		NotifyFontSizeChange();
		return Events::RelevantEvent;
	}
	else if (auto *sassy = Events::ConvertEvent<Event::ReleaseGraphicsEvent>(event))
	{
		return Events::RelevantEvent;
	}
	else if (auto *sassyboi = Events::ConvertEvent<Event::CreateGraphicsEvent>(event))
	{
		return Events::RelevantEvent;
	}
	return Events::IrrelevantEvent;
}

SDLW::Font *Config1::UIConfig::RegisterNewTextFormat(const Stringy& Name, SDLW::FontDesc fd)
{
	DINFO("Attempting registration of new brush named '" + Name + "'");
	
	auto it = RegisteredTextFormats.find(Name);
	if (it != RegisteredTextFormats.end())
	{
		// Exists already
		if (it->second.GetDesc() == fd)
		{
			DINFO("Existing brush is exactly the same");
			return &it->second;
		}
		else
		{
			DINFO("Overwriting existing brush...");
			it->second = SDLW::Font(fd);
			return &it->second;
		}
	}
	else
	{
		// Doesn't exist
		return &RegisteredTextFormats.emplace(Name, SDLW::Font(fd)).first.operator*().second;
	}
}

SDLW::Font *Config1::UIConfig::GetTextFormat(const Stringy& Name)
{
	auto it = RegisteredTextFormats.find(Name);
	if (it == RegisteredTextFormats.end())
		return nullptr;
	return &it->second;
}

SDL_Color Config1::UIConfig::RegisterNewBrush(const Stringy& Name, SDL_Color color)
{
	DINFO("Registering New Brush named '" + Name + "' ");


	auto it = RegisteredBrushs.find(Name);
	if (it == RegisteredBrushs.end())
	{
		RegisteredBrushs.emplace(Name, color);
		DINFO("Registered New Brush '" + Name + "' ");
		return color;
	}
	else
	{
		DINFO("Brush already exists, overwriting...");
		it->second = color;
		return it->second;
	}
}

SDL_Color Config1::UIConfig::GetBrush(const Stringy& Name)
{
	auto it = RegisteredBrushs.find(Name);
	if (it != RegisteredBrushs.end())
		return it->second;
	else
		return { 0, 0, 0, 0 };
}

void Config1::UIConfig::ResizeFonts(float scalechange)
{
	for (auto& pair : RegisteredTextFormats)
	{
		auto& format = pair.second;
		format.ScaleSize(scalechange);
	}
}

bool Event::FontSizeEvent::operator==(Events::IEvent * event)
{
	auto *e = Events::ConvertEvent<FontSizeEvent>(event);
	if (!e)
		return false;

	return (e->FrameID == this->FrameID && e->BigFontSize == BigFontSize && e->NormalFontSize == NormalFontSize && e->SmallFontSize == SmallFontSize);
}

bool Event::MarginChangeEvent::operator==(Events::IEvent * event)
{
	auto *e = Events::ConvertEvent<MarginChangeEvent>(event);
	if (!e)
		return false;

	return (e->FrameID == this->FrameID && e->SmallMargin == SmallMargin && e->NormalMargin == NormalMargin && e->BigMargin == BigMargin);
}