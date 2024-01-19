#include "Core/Debugging/Log.hpp"

#include <iostream>
#include <stdarg.h>
#include <string>
#include <filesystem>
#include "Wrappers/Interfacing.hpp"
#include "Core/App.hpp"

namespace Core::Debugging
{
	std::ofstream logFile;
	bool rawmode = false;
	bool hasError = false;
	bool hasWarnings = false;
	std::string buffer;
	DEBUG_LEVEL verbose = DEBUG_LEVEL::LDEFAULT;

	void Log::OpenLogFile()
	{
		std::filesystem::path p = std::filesystem::current_path().append("Logs");
		if (!std::filesystem::exists(p))
		{
			std::filesystem::create_directory(p);
		}
		std::string name = "Logs/output";
		name.append("@");
		time_t timeLocal;
		struct tm dateTime;
		char text[32];
		time(&timeLocal);
#ifdef _WIN32
		localtime_s(&dateTime, &timeLocal);
#else
		dateTime = *localtime(&timeLocal);
#endif
		strftime(text, 32, "%Y_%m_%d", &dateTime);
		name.append(text);
		name.append(".log");
		if (logFile.is_open()) CloseFile();
		logFile.open(name.c_str(), std::ios::out | std::ios::binary | std::ios::app);
		if (!logFile.is_open())
		{
			std::cerr << "Error: Could not open output file " << name << "\nError Code :" << errno << std::endl;
		}
	}

	void Log::SetRaw()
	{
		rawmode = true;
	}

	void Log::SetVerbosity(DEBUG_LEVEL v)
	{
		verbose = v;
	}

	bool Log::HasError()
	{
		return hasError;
	}

	void Log::ClearErrors()
	{
		hasError = false;
		hasWarnings = false;
	}

	bool Log::HasWarnings()
	{
		return hasWarnings;
	}

	void Log::CloseFile()
	{
		if (!logFile.is_open()) return;
		logFile.close();
	}

	void Log::Print(const char* format, ...)
	{
		va_list args;
		va_start(args, format);
		vprintf(format, args);
		if (!rawmode) putc('\n', stdout);
		va_end(args);
		
		if (logFile.is_open())
		{
			va_start(args, format);
			int size = vsnprintf(nullptr, 0, format, args);
			if (size > static_cast<int>(buffer.size()))
			{
				buffer.resize(size+1llu);
			}
			vsnprintf(buffer.data(), buffer.size(), format, args);
			logFile.write(buffer.c_str(), size);
			auto& it = Core::App::GetInstance()->GetInterfacing();
			it.AddLog(buffer.c_str());
			if (!rawmode)
			{
				logFile.put('\n');
				it.AddLog("\n");
			}
			logFile.flush();
			va_end(args);
		}
		rawmode = false;
	}

	bool isVerboseEnough(DEBUG_LEVEL level)
	{
		return static_cast<char>(level) >= static_cast<char>(verbose);
	}

	void Log::PrintLevel(DEBUG_LEVEL level, const char* format, ...)
	{
		if (!isVerboseEnough(level))
		{
			rawmode = false;
			return;
		}
		switch (level)
		{
		case DEBUG_LEVEL::LDEFAULT:
			if (logFile.is_open()) logFile << "LOG     : ";
			printf("LOG     : ");
			break;
		case DEBUG_LEVEL::LINFO:
			if (logFile.is_open()) logFile << "INFO    : ";
			printf("INFO    : ");
			break;
		case DEBUG_LEVEL::LWARNING:
			if (logFile.is_open()) logFile << "WARNING : ";
			hasWarnings = true;
			printf("WARNING : ");
			break;
		case DEBUG_LEVEL::LERROR:
			if (logFile.is_open()) logFile << "ERROR   : ";
			hasError = true;
			printf("ERROR   : ");
			break;
		default:
			break;
		}
		va_list args;
		va_start(args, format);
		vprintf(format, args);
		if (!rawmode) putc('\n', stdout);
		va_end(args);

		if (logFile.is_open())
		{
			va_start(args, format);
			int size = vsnprintf(nullptr, 0, format, args);
			va_end(args);
			va_start(args, format);
			if (size > static_cast<int>(buffer.size()))
			{
				buffer.resize(size+1llu);
			}
			vsnprintf(buffer.data(), buffer.size(), format, args);
			logFile.write(buffer.c_str(), size);
			auto& it = Core::App::GetInstance()->GetInterfacing();
			it.SetLevel(level);
			it.AddLog(buffer.c_str());
			if (!rawmode)
			{
				logFile.put('\n');
				it.AddLog("\n");
			}
			logFile.flush();
			va_end(args);
		}
		rawmode = false;
	}

	/*
	void Log::glDebugOutput(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
	{
		// ignore non-significant error/warning codes
		if (id == 131169 || id == 131185 || id == 131218 || id == 131204 || id == 131222 || id == 9 || id == 2) return;

		std::cout << "---------------" << std::endl;
		std::cout << "Debug message (" << id << "): " << message << std::endl;

		switch (source)
		{
		case GL_DEBUG_SOURCE_API:             std::cout << "Source: API"; break;
		case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   std::cout << "Source: Window System"; break;
		case GL_DEBUG_SOURCE_SHADER_COMPILER: std::cout << "Source: Shader Compiler"; break;
		case GL_DEBUG_SOURCE_THIRD_PARTY:     std::cout << "Source: Third Party"; break;
		case GL_DEBUG_SOURCE_APPLICATION:     std::cout << "Source: Application"; break;
		case GL_DEBUG_SOURCE_OTHER:           std::cout << "Source: Other"; break;
		} std::cout << std::endl;

		switch (type)
		{
		case GL_DEBUG_TYPE_ERROR:               std::cout << "Type: Error"; break;
		case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: std::cout << "Type: Deprecated Behaviour"; break;
		case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  std::cout << "Type: Undefined Behaviour"; break;
		case GL_DEBUG_TYPE_PORTABILITY:         std::cout << "Type: Portability"; break;
		case GL_DEBUG_TYPE_PERFORMANCE:         std::cout << "Type: Performance"; break;
		case GL_DEBUG_TYPE_MARKER:              std::cout << "Type: Marker"; break;
		case GL_DEBUG_TYPE_PUSH_GROUP:          std::cout << "Type: Push Group"; break;
		case GL_DEBUG_TYPE_POP_GROUP:           std::cout << "Type: Pop Group"; break;
		case GL_DEBUG_TYPE_OTHER:               std::cout << "Type: Other"; break;
		} std::cout << std::endl;

		switch (severity)
		{
		case GL_DEBUG_SEVERITY_HIGH:         std::cout << "Severity: high"; break;
		case GL_DEBUG_SEVERITY_MEDIUM:       std::cout << "Severity: medium"; break;
		case GL_DEBUG_SEVERITY_LOW:          std::cout << "Severity: low"; break;
		case GL_DEBUG_SEVERITY_NOTIFICATION: std::cout << "Severity: notification"; break;
		} std::cout << std::endl;
		std::cout << std::endl;
	}
	*/

#pragma warning(push)
#pragma warning(disable: 26812)
	VKAPI_ATTR VkBool32 VKAPI_CALL Log::debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
	{
		DEBUG_LEVEL level;
		if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
		{
			level = DEBUG_LEVEL::LERROR;
		}
		else if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
		{
			level = DEBUG_LEVEL::LWARNING;
		}
		else if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
		{
			level = DEBUG_LEVEL::LDEFAULT;
		}
		else
		{
			level = DEBUG_LEVEL::LINFO;
		}
		if (!isVerboseEnough(level)) return VK_FALSE;
		std::string type;
		if (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT)
		{
			type += "Generic ";
		}
		if (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT)
		{
			type += "Validation ";
		}
		if (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT)
		{
			type += "Performance ";
		}
		PrintLevel(level, "%sdebug message:", type.c_str());
		PrintLevel(level, pCallbackData->pMessage);
 		return VK_FALSE;
	}
#pragma warning(pop)  
}