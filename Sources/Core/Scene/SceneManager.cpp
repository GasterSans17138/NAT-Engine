#include "Core/Scene/SceneManager.hpp"
#include "Core/App.hpp"
#include "Core/FileManager.hpp"
#include "LowRenderer/RenderPassType.hpp"
#include "LowRenderer/PostProcess/GammaCorrectionPostProcess.hpp"
#include "Wrappers/Sound/SoundEngine.hpp"
#include "../Game/Header/PlayerManager.hpp"

namespace Core::Scene
{
	SceneManager* SceneManager::mInstance = nullptr;

	SceneManager::SceneManager()
	{
		if (!mInstance)
			mInstance = this;
	}

	SceneManager* SceneManager::GetInstance()
	{
		return mInstance;
	}

	void SceneManager::SaveScene(Scene* scene)
	{
		auto& resources = App::GetInstance()->GetResources();
		resources.SaveAsset(scene);
		auto ref = resources.Get<Scene>(scene->hash);
		ref->DeleteData();
		Serialization::Serializer sr;
		scene->Write(sr);
		Serialization::Deserializer dr(sr.GetBuffer(), sr.GetBufferSize());
		u8 type;
		dr.Read(type);
		ref->Load(dr);
	}

	void Core::Scene::SceneManager::DeserializeAllScenes(std::vector<Serialization::Serializer>& scenes)
	{
		u8 type;
		for (u64 i = 0; i < scenes.size(); ++i)
		{
			activeScenes.push_back(new Scene());
			activeScenes.back()->hash = savedSceneData[i].hash;
			activeScenes.back()->isSaved = savedSceneData[i].isSaved;
			Serialization::Deserializer dr(scenes[i].GetBuffer(), scenes[i].GetBufferSize());
			dr.Read(type);
			activeScenes.back()->Load(dr);
			activeScenes.back()->Init();
			activeScenes.back()->ForceUpdate();
		}
	}

	void SceneManager::LoadScene(const std::string file, const SceneLoadingMode& pMode)
	{
		if (pMode == SceneLoadingMode::LOAD_SINGLE)
		{
			for (auto& scene : activeScenes)
				scene->DeleteData();

			activeScenes.clear();
		}

		auto& res = App::GetInstance()->GetResources();
		u64 hash;
		Serialization::Conversion::ToLocal(Maths::Util::ReadHex(file), hash);

		if (!Core::FileManager::FileExist(file.c_str()) || res.IsLoaded(hash)) return;
		auto result = App::GetInstance()->GetResources().Get<Scene>(hash);
		if (!result)
		{
			LOG(DEBUG_LEVEL::LERROR, "Cound not load scene %s !", file.c_str());
			return;
		}
		activeScenes.push_back(result->CreateCopy());
		activeScenes.back()->Init();
		activeScenes.back()->ForceUpdate();
	}

	void SceneManager::UnLoadScene(u64 index)
	{
		if (index >= activeScenes.size()) return;
		activeScenes[index]->DeleteData();
		delete activeScenes[index];
		for (u64 i = index; i < activeScenes.size() - 1; i++)
		{
			activeScenes[i] = activeScenes[i + 1];
		}
		activeScenes.pop_back();
	}

	void SceneManager::Render()
	{
		drawnPortals = 0;
		drawnRecurrence = 0;
		if (clickScene)
		{
			if (clickSceneRendered)
			{
				clickScene = false;
				clickSceneRendered = false;
				const std::string res = renderer->ReadTexture(&renderer->GetMainObjectBuffer(), clickedPos, Maths::IVec2(1, 1), sizeof(u32));
				u32 result;
				std::copy(res.data(), res.data() + 4, reinterpret_cast<char*>(&result));
				if (result && result <= drawnObjects.size())
				{
					clickedObject = drawnObjects[result - 1];
				}
				else
				{
					clickedObject = nullptr;
				}
			}
			else
			{
				clickSceneRendered = true;
				currentCamera = &mainCamera;

				renderer->SetCurrentCamera(&mainCamera);
				Maths::Frustum frustum = mainCamera.CreateFrustumFromCamera();

				renderer->BeginPass(nullptr, LowRenderer::RenderPassType::OBJECT);
				Maths::Mat4 vp = currentCamera->GetProjectionMatrix() * currentCamera->GetViewMatrix();
				for (auto& scene : activeScenes)
				{
					scene->Render(vp, Maths::Mat4(1), frustum, LowRenderer::RenderPassType::OBJECT);
				}

				renderer->EndPass();
			}
		}
		for (auto& camera : registeredCameras)
		{
			renderer->SetStencilState(0, Renderer::StencilState::DEFAULT);
			renderer->currentCameraPos = camera->camera.position;
			currentCamera = &camera->camera;
			clearColor = camera->frameBuffer->ClearColor.GetVector();
			renderer->SetCurrentCamera(&camera->camera);

			renderer->BeginPass(camera->frameBuffer, LowRenderer::RenderPassType::SECONDARY);
			Maths::Mat4 vp = currentCamera->GetProjectionMatrix() * currentCamera->GetViewMatrix();
			Maths::Frustum frustum = currentCamera->CreateFrustumFromCamera();
			for (auto& scene : activeScenes)
			{
				scene->Render(vp, Maths::Mat4(1), frustum, LowRenderer::RenderPassType::SECONDARY);
			}
			u64 counter = 0;
			RenderPortals(*currentCamera, vp, Maths::Mat4(1), Maths::Vec4(-1.0f, -1.0f, 1.0f, 1.0f), counter, 0);
			drawnPortals += counter;
			renderer->EndPass();
			postManager.ApplyEffects(camera->frameBuffer);
		}
		renderer->SetStencilState(0, Renderer::StencilState::DEFAULT);
		renderer->currentCameraPos = mainCamera.position;
		currentCamera = &mainCamera;
		clearColor = renderer->GetClearColor();
		renderer->SetCurrentCamera(&mainCamera);

		renderer->BeginPass();
		Maths::Mat4 vp = currentCamera->GetProjectionMatrix() * currentCamera->GetViewMatrix();
		Maths::Frustum frustum = currentCamera->CreateFrustumFromCamera();
		for (auto& scene : activeScenes)
		{
			scene->Render(vp, Maths::Mat4(1), frustum, LowRenderer::RenderPassType::DEFAULT);
		}

		Resources::ShaderProgram* wire = App::GetInstance()->GetResources().Get<Resources::ShaderProgram>(0x29);
		renderer->BindShader(wire);

		if (ShouldRenderColliders())
			Core::App::GetInstance()->GetPhysicsEngine().EditorRenderColliders();		
		else
			Core::App::GetInstance()->GetPhysicsEngine().EditorClearDrawList();

		u64 counter = 0;
		RenderPortals(*currentCamera, vp, Maths::Mat4(1), Maths::Vec4(-1.0f, -1.0f, 1.0f, 1.0f), counter, 0);
		drawnPortals += counter;

		renderer->EndPass();
		postManager.ApplyEffects(nullptr);
		registeredCameras.clear();
		registeredPortals.clear();
	}

	///Init all the scenes loaded in the manager
	void SceneManager::Init()
	{
		for (auto& scene : activeScenes)
		{
			scene->Init();
			scene->ForceUpdate();
		}
	}	

	bool SceneManager::ShouldRenderColliders()
	{
		return Core::App::GetInstance()->GetInterfacing().IsInitialised() && App::GetInstance()->GetWindow().IsKeyDown(GLFW_KEY_F3);
	}

void SceneManager::Update(Maths::IVec2 resolution, Wrappers::IPhysicsEngine* physicEngine)
{
	if (ifSwapScene)
		SwapScene();
	if (!renderer) renderer = &Core::App::GetInstance()->GetRenderer();
	if (!window) window = &Core::App::GetInstance()->GetWindow();
	renderer->ClearLights();
	if (playMode != PlayMode::EDITION)
	{
		physicEngine->Update();
	}
	else
	{
		physicEngine->ClearEvents();
	}
	for (auto& scene : activeScenes)
	{
		scene->DataUpdate(playMode == PlayMode::EDITION);
	}
	if (playMode != PlayMode::EDITION)
	{
		for (auto& scene : activeScenes)
		{
			scene->Update();
		}
	}
	if (playMode == PlayMode::GAME)
	{
		mainCameraUpReset = false;
		mainCamera.Update(resolution);
	}
	else
	{
		if (!mainCameraUpReset)
		{
			mainCamera.up = Maths::Vec3(0, 1, 0);
			mainCameraUpReset = true;
		}

		Maths::Vec3 delta = mainCamera.position;
		mainCamera.Update(resolution, window);
		App::GetInstance()->GetSoundEngine().UpdateListener(mainCamera.position, mainCamera.GetDeltaUp(), mainCamera.focus - mainCamera.position, (mainCamera.position - delta) / window->GetDeltaTime());
	}
}

	///Clean all scenes currently loaded in the manager
	void SceneManager::Clean()
	{
		for (auto& scene : activeScenes)
		{
			scene->DeleteData();
			delete scene;
		}
		activeScenes.clear();
	}

	void SceneManager::PushRenderCamera(Components::Rendering::CameraComponent* component)
	{
		registeredCameras.push_back(component);
	}

	void SceneManager::PushPortalObject(Components::Rendering::PortalBaseComponent* component)
	{
		registeredPortals.push_back(component);
	}

	PlayMode SceneManager::GetPlayMode() const
	{
		return playMode;
	}

	void SceneManager::SetPlayMode(PlayMode mode)
	{
		playMode = mode;
	}

	bool SceneManager::IsPlaying() const
	{
		return playMode == PlayMode::GAME || playMode == PlayMode::PLAYING;
	}

	bool Core::Scene::SceneManager::HasUnsavedScenes() const
	{
		for (auto& scene : activeScenes)
		{
			if (!scene->isSaved) return true;
		}
		return false;
	}

	void SceneManager::ClickScene(Maths::IVec2 pos)
	{
		clickScene = true;
		clickedPos = pos;
		drawnObjects.clear();
	}

	void SceneManager::ConfigurePostProcesses()
	{
		postManager.ShowGUI();
	}

	GameObject* SceneManager::GetClickedObject() const
	{
		return clickedObject;
	}

	void SceneManager::SetClickedObject(GameObject* object)
	{
		clickedObject = object;
	}

	u32 SceneManager::GetNextIndex(GameObject* object)
	{
		drawnObjects.push_back(object);
		return static_cast<u32>(drawnObjects.size());
	}

	Scene* SceneManager::GetGameObjectScene(GameObject* object)
	{
		while (object->parent) object = object->parent;
		for (auto& scene : activeScenes)
		{
			for (auto& gameObject : scene->gameObjects)
			{
				if (gameObject == object)
				{
					return scene;
				}
			}
		}
		LOG(DEBUG_LEVEL::LERROR, "Could not find scene for GameObject %s!", object->name.c_str());
		return nullptr;
	}

	LowRenderer::Rendering::Camera* SceneManager::GetMainCamera()
	{
		return reinterpret_cast<LowRenderer::Rendering::Camera*>(&mainCamera);
	}

	void SceneManager::SwapScene()
	{
		App::GetInstance()->GetSceneManager().SetPlayMode(Core::Scene::PlayMode::EDITION);
		auto& sceneM = App::GetInstance()->GetSceneManager();
		auto& scenes = sceneM.activeScenes;
		scenes.push_back(reinterpret_cast<Core::Scene::Scene*>(App::GetInstance()->GetResources().GetSceneList()[idScenetoLoad])->CreateCopy());
		scenes.back()->Init();
		scenes.back()->ForceUpdate();
		sceneM.UnLoadScene(0);
		App::GetInstance()->GetSceneManager().SetPlayMode(Core::Scene::PlayMode::GAME);
		App::GetInstance()->GetWindow().SetCursorCaptured(false);
		ifSwapScene = false;
	}

	std::vector<Serialization::Serializer> SceneManager::SerializeAllScenes()
	{
		savedSceneData.clear();
		std::vector<Serialization::Serializer>savedScenes;
		savedScenes.reserve(activeScenes.size());
		
		for (auto& scene : activeScenes)
		{
			SceneSaveData data;
			data.hash = scene->hash;
			data.isSaved = scene->isSaved;
			savedSceneData.push_back(std::move(data));
			Serialization::Serializer sr;
			scene->Write(sr);
			savedScenes.push_back(sr);
		}

		return savedScenes;
	}

	void SceneManager::RenderPortals(const LowRenderer::Rendering::Camera& cam, const Maths::Mat4& vp, const Maths::Mat4& m, Maths::Vec4 screenBounds, u64& drawnPortals, u8 recurrence)
	{
		if (drawnRecurrence < recurrence) drawnRecurrence = recurrence;
		for (auto& portal : registeredPortals)
		{
			if (drawnPortals >= maxPortalCount)
			{
				return;
			}
			if (!portal->IsVisibleOnScreen(&cam)) continue;
			Maths::Mat4 mvp = vp * portal->gameObject->transform.GetGlobal();
			Maths::Vec4 bounds = portal->GetScreenCovering(mvp);
			bounds = bounds.Clip(screenBounds);
			Maths::Vec2 area = Maths::Vec2(bounds.z - bounds.x, bounds.w - bounds.y);
			if (area.x <= 0.0f || area.y <= 0.0f) continue;
			Maths::Vec4 nearPlane;
			Maths::Mat4 model = m;
			LowRenderer::Rendering::Camera cam2 = portal->UpdateCamera(cam, model, nearPlane);
			++drawnPortals;
			portal->Overwrite(mvp, Renderer::StencilState::INCREMENT, recurrence, clearColor);
			portal->Overwrite(mvp, Renderer::StencilState::NO_DEPTH, recurrence + 1, clearColor);
			Maths::Mat4 vp2;
			if (nearPlane.w < 1.0f && (portal->gameObject->transform.GetGlobal().GetPositionFromTranslation() - cam.position).GetLength() < 4.0f)
			{
				vp2 = cam2.GetProjectionMatrix() * cam2.GetViewMatrix();
			}
			else
			{
				vp2 = cam2.GetObliqueProjection(nearPlane) * cam2.GetViewMatrix();
			}
			auto oldCam = currentCamera;
			currentCamera = &cam2;
			renderer->SetCurrentCamera(&cam2);
			renderer->SetStencilState(recurrence + 1, Renderer::StencilState::DEFAULT);
			Maths::Frustum frustum = cam2.CreateFrustumFromCamera();
			for (auto& scene : activeScenes)
			{
				scene->Render(vp2, model, frustum, static_cast<LowRenderer::RenderPassType>(LowRenderer::RenderPassType::DEFAULT | LowRenderer::RenderPassType::SECONDARY));
			}
			currentCamera = oldCam;
			renderer->SetCurrentCamera(oldCam);
			if (recurrence < maxPortalRecurrence)
			{
				RenderPortals(cam2, vp2, model, bounds, drawnPortals, recurrence + 1);
			}
			portal->Overwrite(mvp, Renderer::StencilState::GREATER, recurrence, clearColor);
		}
	}
}