#include "Core/FileManager.hpp"

#include <fstream>
#include <filesystem>
#include <errno.h>

#ifndef _WIN32
#include <cstring>
#endif

#include "Core/Debugging/Log.hpp"

using namespace Core;

template<typename T>
T Load(const char* path)
{
	std::ifstream in;
	in.open(path, std::ios::binary | std::ios::in | std::ios::ate);
	if (!in.is_open())
	{
#ifdef _WIN32
		std::string result = std::string();
		result.resize(256);
		strerror_s(result.data(), result.size(), errno);
		LOG(DEBUG_LEVEL::LWARNING, "Could not open file %s : Error code %d", path, result.c_str());
#else
		LOG(DEBUG_LEVEL::LWARNING, "Could not open file %s : Error code %d", path, strerror(errno));
#endif
		return T();
	}
	u64 filelength = in.tellg();
	in.seekg(0, in.beg);

	T result;
	result.resize(filelength);
	in.read(reinterpret_cast<char*>(result.data()), filelength);
	in.close();
	return result;
}

std::string FileManager::LoadFile(const char* path)
{
	return Load<std::string>(path);
}

std::vector<u8> FileManager::LoadFileAsVector(const char* path)
{
	return Load<std::vector<u8>>(path);
}

bool Core::FileManager::FileExist(const char* path)
{
	return std::filesystem::exists(path);
}

template<typename T>
bool Write(const char* path, const T& content)
{
	std::ofstream out;
	out.open(path, std::ios::binary | std::ios::trunc);
	if (!out.is_open()) return false;
	out.write(reinterpret_cast<const char*>(content.data()), content.size());
	out.close();
	return true;
}

bool Core::FileManager::WriteFile(const char* path, const std::string& content)
{
	return Write(path, content);
}

bool Core::FileManager::WriteFile(const char* path, const std::vector<u8>& content)
{
	return Write(path, content);
}

void Core::FileManager::WriteFile(const char* path, const u8* content, u64 contentSize)
{
	std::ofstream out;
	out.open(path, std::ios::binary | std::ios::trunc);
	if (!out.is_open()) return LOG(DEBUG_LEVEL::LWARNING, "Could not open file %s", path);
	out.write(reinterpret_cast<const char*>(content), contentSize);
	out.close();
}