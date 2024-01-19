#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <fstream>

#ifdef NAT_EngineDLL
#define NAT_API __declspec(dllexport)
#else
#define NAT_API __declspec(dllimport)
#endif // NAT_EngineDLL

enum class DEBUG_LEVEL : char
{
	LINFO = 0,
	LDEFAULT = 1,
	LWARNING,
	LERROR,
};

#ifdef _WIN32
#define DEBUG_LOG(x, ...) printf("[%s:%d] ",__FILE__,__LINE__); printf(x, __VA_ARGS__)
#define LOG(level, x, ...) Core::Debugging::Log::PrintLevel(level, x, __VA_ARGS__)
#define LOGRAW(x, ...) Core::Debugging::Log::SetRaw(); Core::Debug::Log::Print(x, __VA_ARGS__)
#define LOG_DEFAULT(x, ...) Core::Debugging::Log::PrintLevel(DEBUG_LEVEL::LDEFAULT, x, __VA_ARGS__)
#else
#define VA_ARGS(...) __VA_OPT__(,) ##__VA_ARGS__
#define DEBUG_LOG(x, ...) printf("[%s:%d] ",__FILE__,__LINE__); printf(x VA_ARGS(__VA_ARGS__))
#define LOG(level, x, ...) Core::Debugging::Log::PrintLevel(level, x VA_ARGS(__VA_ARGS__))
#define LOGRAW(x, ...) Core::Debugging::Log::SetRaw(); Core::Debug::Log::Print(x VA_ARGS(__VA_ARGS__))
#define LOG_DEFAULT(x, ...) Core::Debugging::Log::PrintLevel(DEBUG_LEVEL::LDEFAULT, x VA_ARGS(__VA_ARGS__))
#endif

namespace Core::Debugging::Log
{
	NAT_API void OpenLogFile();
	NAT_API	void CloseFile();
	NAT_API bool HasError();
	NAT_API void Print(const char* format, ...);
	NAT_API void PrintLevel(DEBUG_LEVEL level, const char* format, ...);
	NAT_API void SetRaw();
	NAT_API void SetVerbosity(DEBUG_LEVEL v);
	NAT_API bool HasError();
	NAT_API void ClearErrors();
	NAT_API bool HasWarnings();

	//void APIENTRY glDebugOutput(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam);
	NAT_API VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);
}