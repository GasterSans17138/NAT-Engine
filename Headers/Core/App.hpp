#pragma once

#include <memory>

#include "Renderer/VulkanRenderer.hpp"

#include "Scene/SceneManager.hpp"

#include "Wrappers/Interfacing.hpp"
#include "Wrappers/WindowManager.hpp"
#include "Wrappers/PhysicsEngine/JoltPhysicsEngine.hpp"
#include "Resources/ResourceManager.hpp"
#include "Wrappers/Sound/SoundEngine.hpp"
#include "Wrappers/Sound/SoundLoader.hpp"

namespace Core
{
	class NAT_API App
	{
	public:
		App();
		~App() = default;

		virtual void Init() = 0;
		virtual void Run() = 0;
		virtual void Destroy() = 0;

		Maths::IVec2 defaultResolution = Maths::IVec2(800, 600);
		Scene::Scene* defaultScene = nullptr;
		u64 defaultSceneHash = 0x69;
		std::vector<u8> postProcesssData;

		Renderer::VulkanRenderer& GetRenderer();
		Wrappers::Interfacing& GetInterfacing();
		Resources::ResourceManager& GetResources();
		Wrappers::WindowManager& GetWindow();
		Wrappers::Sound::SoundEngine& GetSoundEngine();
		Wrappers::Sound::SoundLoader& GetSoundLoader();
		Scene::SceneManager& GetSceneManager();
		Wrappers::PhysicsEngine::JoltPhysicsEngine& GetPhysicsEngine();
		static App* GetInstance();
		void TakeScreenshot();

	protected:
		Resources::ResourceManager resourceManager;
		Renderer::VulkanRenderer renderer;
		Wrappers::WindowManager window;
		Wrappers::Interfacing guiInterface;
		Wrappers::PhysicsEngine::JoltPhysicsEngine physicsEngine;
		Wrappers::Sound::SoundEngine soundEngine;
		Wrappers::Sound::SoundLoader soundLoader;
		
		Scene::SceneManager sceneManager;


		static inline App* appInstance = nullptr;

		void CreateFolders();
		void LoadResourcesAlreadyCached();
		bool LoadProjectSettings();
		void SaveProjectSettings();
	private: 
		virtual void DefaultSamplerLoaded();
	};
}