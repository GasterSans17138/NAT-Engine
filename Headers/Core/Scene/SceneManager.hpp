#pragma once

#include <vector>

#include "Scene.hpp"
#include "LowRenderer/Rendering/FlyingCamera.hpp"
#include "Components/Rendering/CameraComponent.hpp"
#include "Components/Rendering/PortalBaseComponent.hpp"
#include "LowRenderer/PostProcess/PostProcessManager.hpp"
#include "Wrappers/PhysicsEngine/IPhysicsEngine.hpp"

namespace Renderer
{
	class VulkanRenderer;
}

#ifdef NAT_EngineDLL
	#define NAT_API __declspec(dllexport)
#else
	#define NAT_API __declspec(dllimport)
#endif // NAT_EngineDLL


namespace Core
{
	class App;

	namespace Scene
	{
		enum class PlayMode : u8
		{
			EDITION = 0,
			PLAYING,
			GAME,
		};

		struct SceneSaveData
		{
			u64 hash;
			bool isSaved;
		};

		enum SceneLoadingMode
		{
			LOAD_SINGLE,	//Will unload all scenes current loaded and load the new one
			LOAD_ADD		//Add the scene to the one(s) currently loaded
		};

		class NAT_API SceneManager
		{
		public:
			SceneManager();
			~SceneManager() = default;

			static SceneManager* GetInstance();

			void SaveScene(Scene* scene);
			std::vector<Serialization::Serializer> SerializeAllScenes();

			void DeserializeAllScenes(std::vector<Serialization::Serializer>& scenes);
			void LoadScene(const std::string file, const SceneLoadingMode& pMode = LOAD_SINGLE);

			void UnLoadScene(u64 index);

			void Init();
			void Render();
			void Update(Maths::IVec2 resolution, Wrappers::IPhysicsEngine* pPhysicEngine);
			void Clean();

			void PushRenderCamera(Components::Rendering::CameraComponent* component);
			void PushPortalObject(Components::Rendering::PortalBaseComponent* component);
			bool IsPlaying() const;
			bool HasUnsavedScenes() const;

			PlayMode GetPlayMode() const;
			void SetPlayMode(PlayMode mode);
			void ClickScene(Maths::IVec2 pos);
			void ConfigurePostProcesses();
			GameObject* GetClickedObject() const;
			void SetClickedObject(GameObject* object);
			u32 GetNextIndex(GameObject* object);
			Scene* GetGameObjectScene(GameObject* object);
			LowRenderer::Rendering::Camera* GetMainCamera();
			bool ifSwapScene = false;

			void SwapScene();
			int idScenetoLoad;
		public:
			std::vector<Scene*> activeScenes;
			LowRenderer::Rendering::Camera* currentCamera = nullptr;
			u64 selectedScene = ~0;
			u8 maxPortalRecurrence = 2;
			u64 maxPortalCount = 128;
			u64 drawnPortals = 0;
			u8 drawnRecurrence = 0;
			LowRenderer::PostProcess::PostProcessManager postManager;
		private:
			LowRenderer::Rendering::FlyingCamera mainCamera;
			PlayMode playMode = PlayMode::EDITION;
			std::vector<Components::Rendering::CameraComponent*> registeredCameras;
			std::vector<Components::Rendering::PortalBaseComponent*> registeredPortals;
			std::vector<SceneSaveData> savedSceneData;
			Renderer::VulkanRenderer* renderer = nullptr;
			Wrappers::WindowManager* window = nullptr;
			GameObject* clickedObject = nullptr;
			bool clickScene = false;
			bool clickSceneRendered = false;
			bool mainCameraUpReset = false;
			Maths::IVec2 clickedPos;
			std::vector<GameObject*> drawnObjects;
			Maths::Vec3 clearColor;
			static SceneManager* mInstance;

			void RenderPortals(const LowRenderer::Rendering::Camera& cam, const Maths::Mat4& vp, const Maths::Mat4& m, Maths::Vec4 screenBounds, u64& drawnPortals, u8 recurrence);
			bool ShouldRenderColliders();

			friend Core::App;
		};
	}
}