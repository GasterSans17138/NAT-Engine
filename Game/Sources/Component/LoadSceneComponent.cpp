#include "Core/App.hpp"
#include "..\..\Header\Component\LoadSceneComponent.hpp"

namespace Core::Scene::Components::Game
{
	IComponent* LoadSceneComponent::CreateCopy()
	{
		return new LoadSceneComponent(*this);
	}

	void LoadSceneComponent::RenderGui()
	{
		auto& gui = Core::App::GetInstance()->GetInterfacing();
		gui.DragInt("Scene To load", &idScene);
		//TODO : Expose loading mode
	}

	ComponentType LoadSceneComponent::GetType()
	{
		return ComponentType::LoadSceneComponent;
	}

	void LoadSceneComponent::Serialize(Core::Serialization::Serializer& sr) const
	{
		sr.Write(idScene);
	}

	void LoadSceneComponent::Deserialize(Core::Serialization::Deserializer& dr)
	{
		dr.Read(idScene);
	}

	void LoadSceneComponent::OnCollisionBegin(Colliders::ICollider* pCollider, GameObject* pOther, Colliders::ICollider* pOtherCollider, Maths::Vec3 normal)
	{
		App::GetInstance()->GetSceneManager().idScenetoLoad = idScene;
		App::GetInstance()->GetSceneManager().ifSwapScene = true;
	}

	const char* LoadSceneComponent::GetName()
	{
		return "LoadSceneComponent";
	}
}
