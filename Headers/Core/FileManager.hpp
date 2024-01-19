#pragma once

#include <string>
#include <vector>

#include "Types.hpp"

#ifdef NAT_EngineDLL
	#define NAT_API __declspec(dllexport)
#else
	#define NAT_API __declspec(dllimport)
#endif // NAT_EngineDLL

namespace Core::FileManager
{
	NAT_API std::string LoadFile(const char* path);
	NAT_API std::vector<u8> LoadFileAsVector(const char* path);

	NAT_API bool FileExist(const char* path);

	NAT_API bool WriteFile(const char* path, const std::string& content);
	NAT_API bool WriteFile(const char* path, const std::vector<u8>& content);
	NAT_API void WriteFile(const char* path, const u8* content, u64 contentSize);
}