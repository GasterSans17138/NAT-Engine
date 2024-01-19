#include "Core/GameApp.hpp"

#include <iostream>
#include <stdexcept>

#include "Core/Debugging/Log.hpp"
#include "Core/Debugging/Assert.hpp"

#ifdef _WIN32
#ifndef NDEBUG
#include <crtdbg.h>
#endif

#ifdef NDEBUG
#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")
#endif
#endif

/*
* TODO : Unloading materials cause vulkan validation errors
*/

s32 main(s32 argc, char** argv)
{
#ifndef NDEBUG
#ifdef _WIN32
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	//_CrtSetBreakAlloc(7818);
#endif
	//try
#endif
	{
		Wrappers::PhysicsEngine::JoltPhysicsEngine::Register();

		Core::Debugging::Log::SetVerbosity(DEBUG_LEVEL::LDEFAULT);
		Core::Debugging::Log::OpenLogFile(); 
		Core::GameApp app;
		app.Init();
		app.Run();
		app.Destroy();
	}
#ifndef NDEBUG
	//catch (const std::exception& e)
	{
		//Wrappers::WindowManager::ErrorSound();
		//Wrappers::WindowManager::OpenPopup("Fatal error", e.what(), Wrappers::PopupParam::BUTTON_OK | Wrappers::PopupParam::ICON_WARNING);
		//std::cerr << e.what() << std::endl;
		//return EXIT_FAILURE;
	}
#endif
#ifdef _WIN32
	if (Core::Debugging::Log::HasError()) system("pause");
#endif
}