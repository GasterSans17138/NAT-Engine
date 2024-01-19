#pragma once
#include "Core/Scene/Components/IComponent.hpp"

namespace Core::Scene::Components::Game
{

	class NAT_API LoadSceneComponent final : public IComponent
	{
	public:
		LoadSceneComponent()	= default;
		~LoadSceneComponent()	= default;

		void RenderGui()			override;
		ComponentType GetType()		override;
		IComponent* CreateCopy()	override;

		virtual void Deserialize(Core::Serialization::Deserializer& dr) override;
		virtual void Serialize(Core::Serialization::Serializer& sr) const override;

		virtual void OnCollisionBegin(Colliders::ICollider* pCollider, GameObject* pOther, Colliders::ICollider* pOtherCollider, Maths::Vec3 normal) override;

		const char* GetName() override;

	public : 
		void LoadScene();
		void SetSceneToLoad(const std::string& pSceneName);

	private :
		//std::string mSceneToLoad = "";
		int idScene = 0;
		Core::Scene::SceneLoadingMode mLoadingMode = Core::Scene::SceneLoadingMode::LOAD_SINGLE;
	};
}