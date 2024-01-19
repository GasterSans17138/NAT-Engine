#include "Core/GameApp.hpp"
#include "Wrappers/ImageLoader.hpp"

namespace Core
{
	void GameApp::Init()
	{
		CreateFolders();
		bool isFullScreen = LoadProjectSettings();
		window.CreateWindow(APP_NAME, defaultResolution);
		window.SetKeyboardEnable(true);
		window.SetMouseButtonEnable(true);
		window.SetMouseScrollEnable(true);
		window.SetFileDropEnable(false);
		window.SetupCallbacks();
		renderer.enableValidationLayers = false;
		renderer.PreInit();
		renderer.SetClearColor(Maths::Vec4(powf(0.57f, 2.4f), powf(0.84f, 2.4f), powf(0.95f, 2.4f), 1.0f));
		window.CreateSurface(renderer.GetInstance(), renderer.GetSurface());
		renderer.Init(static_cast<GLFWwindow*>(window.GetWindowPointer()), defaultResolution, true);
		window.SetFullScreen(isFullScreen);
		soundEngine.Init();

		physicsEngine.Init(&renderer);
		sceneManager.postManager.Init();
		//resourceManager.EnableAutoDelete();
		LoadResourcesAlreadyCached();
		Serialization::Deserializer dr(postProcesssData);
		sceneManager.postManager.Deserialize(dr);
		defaultScene = resourceManager.Get<Scene::Scene>(defaultSceneHash);
		if (!defaultScene) defaultScene = resourceManager.Get<Scene::Scene>(0x69);
		sceneManager.SetPlayMode(Scene::PlayMode::GAME);
		sceneManager.activeScenes.push_back(defaultScene->CreateCopy());
		sceneManager.activeScenes.back()->Init();
		sceneManager.activeScenes.back()->ForceUpdate();

		auto vector = window.GetAllScreenMaxFrameRates();
		f32 maximum = 0;
		for (auto& value : vector)
		{
			maximum = Maths::Util::MaxF(maximum, value);
		}
		window.SetTargetFrameRate(maximum);
	}

	void GameApp::Run()
	{
		renderer.WaitForVSync();
		
		//window.SetCursorCaptured(true);

		while (!window.ShouldClose())
		{
			//----- Update -----
			window.Update();

			if (window.IsKeyPressed(GLFW_KEY_F11))
				window.SetFullScreen(!window.IsFullScreen());

			if (window.IsKeyPressed(GLFW_KEY_ESCAPE))
				window.SetShouldClose(true);

			Maths::IVec2 res = window.GetWindowSize();
			renderer.ResizeFrameBuffer(res);
			if (window.IsKeyPressed(GLFW_KEY_F2))
			{
				appInstance->TakeScreenshot();
			}

			if (!window.IsVisible())
			{
#ifdef _WIN32
				std::this_thread::sleep_for(std::chrono::milliseconds(1));
#else
				std::this_thread::sleep_for(std::chrono::microseconds(16666 - static_cast<s64>(window.GetDeltaTime() * 1000000llu)));
#endif
				continue; //Next frame
			}
			sceneManager.Update(res, &physicsEngine);
			//------------------


			//----- Render -----
			renderer.BeginFrame(nullptr);

			sceneManager.Render();

			renderer.EndFrame(nullptr);
			//------------------
			renderer.WaitForVSync();
		}
	}

	void GameApp::Destroy()
	{
		SaveProjectSettings();
		sceneManager.Clean();
		renderer.WaitIdle();
		resourceManager.DeleteAllResources();
		renderer.CleanupBuffers();
		soundEngine.Shutdown();

		physicsEngine.Release();

		renderer.Cleanup();
	}
}