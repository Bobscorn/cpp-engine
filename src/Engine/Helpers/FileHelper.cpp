#include "FileHelper.h"

#ifdef __linux__
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
#else
#include <filesystem>
namespace fs = std::filesystem;
#endif

#include <fstream>

std::string FileHelper::WorkingDirectory()
{
	return fs::current_path().string();
}

std::vector<std::string> FileHelper::GetDirectoryFiles(std::string directory)
{
	std::vector<std::string> out;

	try
	{
		for (const auto &p : fs::directory_iterator(directory))
		{
			if (fs::is_regular_file(p.status()))
			{
				out.emplace_back(p.path().string());
			}
		}
	}
	catch (const fs::filesystem_error &e)
	{
		(void)e;
		return {};
	}

	return out;
}

std::string FileHelper::GetFileRaw(std::string path)
{
	if (!FileExists(path))
		return "";

	std::ifstream in{ path, std::ifstream::in };

	std::string out;
	in.seekg(0, in.end);
	size_t length = in.tellg();
	out.resize(length);
	in.seekg(0, in.beg);
	in.read(out.data(), length);
	in.close();

	return out;
}

std::vector<char> FileHelper::GetFileRawV(std::string path)
{
	if (!FileExists(path))
		return {};

	std::ifstream in{ path, std::ifstream::in };

	std::vector<char> out;
	in.seekg(0, in.end);
	size_t length = in.tellg();
	out.resize(length);
	in.seekg(0, in.beg);
	in.read(out.data(), length);
	in.close();

	return out;
}

bool FileHelper::DirectoryExists(std::string path)
{
	if (fs::exists(path))
		return fs::is_directory(path);
	return false;
}

bool FileHelper::FileExists(std::string path)
{
	if (fs::exists(path))
		return fs::is_regular_file(path);
	return false;
}

bool FileHelper::PathExists(std::string path)
{
	return fs::exists(path);
}
