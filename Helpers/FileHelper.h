#pragma once

#include <string>
#include <vector>
namespace FileHelper
{
	std::string WorkingDirectory();
	std::vector<std::string> GetDirectoryFiles(std::string directory);
	std::string GetFileRaw(std::string path);
	std::vector<char> GetFileRawV(std::string path);

	bool DirectoryExists(std::string path);
	bool FileExists(std::string path);
	bool PathExists(std::string path);
}