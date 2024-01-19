#pragma once

#include <vector>
#include <string>

#include "GameObject.hpp"
#include "Resources/IResource.hpp"

#include "Core/Serialization/Serializer.hpp"
#include "Core/Serialization/Deserializer.hpp"

#define DEFAULTSCENE_NAME "Default Scene"


#ifdef NAT_EngineDLL
#define NAT_API __declspec(dllexport)
#else
#define NAT_API __declspec(dllimport)
#endif // NAT_EngineDLL

namespace Core::Scene
{
	class SceneManager;

	class NAT_API Scene : public Resources::IResource
	{
	public:
		Scene();
		~Scene() override;

		void DeleteData() override;
		void Write(Core::Serialization::Serializer& sr) override;
		void Load(Core::Serialization::Deserializer& dr) override;
		void WindowCreateResource(bool& open) override;
		Resources::ObjectType GetType() override;
		Scene* CreateCopy();

		void Init();
		void Update();
		void DataUpdate(bool updateTransform);
		void Render(const Maths::Mat4& vp, const Maths::Mat4& modelOverride, const Maths::Frustum& frustum, LowRenderer::RenderPassType renderPass);
		void ForceUpdate();
		GameObject* AddChild();

		std::vector<GameObject*> gameObjects;
		bool isNodeOpen = false;
		SceneManager* scenes = nullptr;
		bool prevbool = false;
		Scene* newScene = nullptr;
	};
}