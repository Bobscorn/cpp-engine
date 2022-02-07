#pragma once

#include "Importer.h"

namespace Config1
{
	struct WindowConfig
	{
		const static ::Stringy DefaultWindowConfigFile;

		int Width = 800, Height = 600;
		int X = 200, Y = 200;
		bool FullScreen = false;
		bool Maximized = false;
		char const *Title = "Encapsulated";

		WindowConfig() {}
		WindowConfig(Stringy workdir, Stringy Filename = DefaultWindowConfigFile) { Reset(workdir + Filename); };
		WindowConfig(std::unordered_map<Importing::XMLKey, Importing::bob<Importing::XMLElement>> &xmlbits) { Reset(xmlbits); };

		void Reset(::Stringy Filename);
		void Reset(std::unordered_map<Importing::XMLKey, Importing::bob<Importing::XMLElement>> &xmlbits);

		void WriteToFile(Stringy workdir, ::Stringy FileName = DefaultWindowConfigFile);
	};
}