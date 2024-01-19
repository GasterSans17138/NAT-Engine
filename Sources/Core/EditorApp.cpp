#include "Core/EditorApp.hpp"

#include "Wrappers/ShaderLoader.hpp"
#include "Wrappers/ImageLoader.hpp"

namespace Core
{
	void EditorApp::Init()
	{
		CreateFolders();
		bool isFullScreen = LoadProjectSettings();
		window.CreateWindow(APP_NAME, defaultResolution);
		window.SetKeyboardEnable(true);
		window.SetMouseButtonEnable(true);
		window.SetMouseScrollEnable(true);
		window.SetFileDropEnable(true);
		window.SetupCallbacks();
		renderer.PreInit();
		renderer.SetClearColor(Maths::Vec4(powf(0.57f, 2.4f), powf(0.84f, 2.4f), powf(0.95f, 2.4f), 1.0f));
		window.CreateSurface(renderer.GetInstance(), renderer.GetSurface());
		renderer.Init(static_cast<GLFWwindow*>(window.GetWindowPointer()), defaultResolution, true, true);
		window.SetFullScreen(isFullScreen);
		soundEngine.Init();

		physicsEngine.Init(&renderer);
		sceneManager.postManager.Init();
		LoadResourcesAlreadyCached();
		Serialization::Deserializer dr(postProcesssData);
		sceneManager.postManager.Deserialize(dr);
		defaultScene = resourceManager.Get<Scene::Scene>(defaultSceneHash);
		if (!defaultScene) defaultScene = resourceManager.Get<Scene::Scene>(0x69);
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

	void EditorApp::Run()
	{
		physicsEngine.SetGravity(Maths::Vec3(0, -15, 0));
		renderer.WaitForVSync();
		physicsEngine.SetGravity(Maths::Vec3(0, -15, 0));
		while (!window.ShouldClose())
		{
			window.Update();
			if (window.ShouldClose() && sceneManager.HasUnsavedScenes())
			{
				if (window.OpenPopup("Warning", "Some of the scenes are unsaved, are you sure you want to quit ?", Wrappers::PopupParam::BUTTON_YES_NO | Wrappers::PopupParam::ICON_WARNING) != 6)
				{
					window.SetShouldClose(false);
				}
			}

			if (window.IsKeyPressed(GLFW_KEY_F11))
				window.SetFullScreen(!window.IsFullScreen());

			guiInterface.UpdateFps(window.GetDeltaTime(), static_cast<u64>(window.GetWindowTime()));
			guiInterface.EngineInterface();
			soundEngine.Update(&guiInterface);

			if (!window.IsVisible())
			{
				guiInterface.Render();
#ifdef _WIN32
				std::this_thread::sleep_for(std::chrono::milliseconds(1));
#else
				std::this_thread::sleep_for(std::chrono::microseconds(16666 - static_cast<s64>(window.GetDeltaTime() * 1000000llu)));
#endif
				continue; //Next frame
			}

			bool extra_color = sceneManager.IsPlaying();
			if (extra_color) guiInterface.PushBGColor(Maths::Vec3(1.0f,0.2f,0.2f));
			guiInterface.Begin("Game View");
			if (sceneManager.GetPlayMode() == Core::Scene::PlayMode::EDITION || sceneManager.GetPlayMode() == Core::Scene::PlayMode::PLAYING)
			{
				if (window.IsKeyPressed(GLFW_KEY_TAB) || (window.IsCursorCaptured() && window.IsKeyPressed(GLFW_KEY_ESCAPE)))
				{
					window.SetCursorCaptured(!window.IsCursorCaptured());
					guiInterface.SetWindowFocused();
				}
			}
			else if (window.IsKeyPressed(GLFW_KEY_TAB))
			{
				window.SetCursorCaptured(false);
				guiInterface.SetWindowFocused();
				sceneManager.SetPlayMode(Scene::PlayMode::PLAYING);
			}
			Maths::IVec2 res = guiInterface.GetWindowSize();
			renderer.ResizeFrameBuffer(res);
			if (window.IsKeyPressed(GLFW_KEY_F2))
			{
				appInstance->TakeScreenshot();
			}
			sceneManager.Update(res, &physicsEngine);
			
			renderer.BeginFrame(&guiInterface);
			sceneManager.Render();
			auto fb = renderer.GetMainLightBuffer();
			guiInterface.Image(fb.GetRendererTexture(), fb.GetResolution());

			if (guiInterface.IsClicked())
			{
				Maths::IVec2 pos = window.GetCursorPos() + window.GetWindowPos() - guiInterface.GetWindowPos() - guiInterface.GetWindowPadding();
				sceneManager.ClickScene(pos);
			}
			if (sceneManager.GetClickedObject())
			{
				guiInterface.SetSelectedObject(sceneManager.GetClickedObject());
				sceneManager.SetClickedObject(nullptr);
			}
			guiInterface.End();
			if (extra_color) guiInterface.PopBGColor();
			guiInterface.Render();

			renderer.EndFrame(&guiInterface);
			renderer.WaitForVSync();
		}
	}

	void EditorApp::Destroy()
	{
		SaveProjectSettings();
		sceneManager.Clean();
		renderer.WaitIdle();
		resourceManager.DeleteAllResources();
		renderer.CleanupBuffers();
		soundEngine.Shutdown();

		physicsEngine.Release();

		guiInterface.Destroy();
		renderer.Cleanup();
	}

	void EditorApp::DefaultSamplerLoaded()
	{
		guiInterface.InitImgui(static_cast<GLFWwindow*>(window.GetWindowPointer()));
	}
};