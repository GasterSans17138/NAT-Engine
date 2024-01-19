#pragma once

#include <vector>
#include <memory>
#include <string>

#include "Core/Debugging/Assert.hpp"
#include "Core/Debugging/Log.hpp"
#include "Components/IComponent.hpp"
#include "Transform.hpp"
#include "Core/Serialization/Serializer.hpp"
#include "Core/Serialization/Deserializer.hpp"

#define GAMEOBJECT_DEFAULT_NAME "GameObject"

#ifdef NAT_EngineDLL
	#define NAT_API __declspec(dllexport)
#else
	#define NAT_API __declspec(dllimport)
#endif // NAT_EngineDLL

namespace Core::Scene
{
	class Scene;

	class NAT_API GameObject
	{
	public:
		GameObject(GameObject* pParent = nullptr);
		GameObject(const GameObject& other);
		~GameObject();

		void Init();
		void Update();
		void DataUpdate(bool updateTransform);
		void Render(const Maths::Mat4& vp, const Maths::Mat4& modelOverride, const Maths::Frustum& frustum, LowRenderer::RenderPassType pass);
		void Delete();

		void OnCollisionBegin(Components::Colliders::ICollider* pCollider, GameObject* pOther, Components::Colliders::ICollider* pOtherCollider, Maths::Vec3 normal);
		void OnCollisionStay(Components::Colliders::ICollider* pCollider, GameObject* pOther, Components::Colliders::ICollider* pOtherCollider, Maths::Vec3 normal);
		void OnCollisionEnd(Components::Colliders::ICollider* pCollider, GameObject* pOther, Components::Colliders::ICollider* pOtherCollider);

		GameObject* Find(std::string pName) const;
		void CopyTo(GameObject* dest, GameObject* parent = nullptr);
		
		template<class T> inline T* AddComponent();

		void SetActive(const bool& pState);
		bool IsActive() const;

		GameObject* GetParent();
		GameObject* AddChild();
		GameObject* AddChild(GameObject* pChildObject);
		GameObject& operator=(const GameObject& other);

		void Serialize(Core::Serialization::Serializer& sr) const;
		void Deserialize(Core::Serialization::Deserializer& dr);
		void ForceUpdate();

		Components::IComponent* GetComponent(Components::ComponentType type);

	public:
		std::string name = GAMEOBJECT_DEFAULT_NAME;

		GameObject* parent = nullptr;
		std::vector<Components::IComponent*> components;
		std::vector<GameObject*> childs;

		Transform transform;

		bool isNodeOpen = false;
		bool isModif = false;
	private:
		bool mIsActive = true;
		void Clean();

		friend Scene;
	};
}

#include "GameObject.inl"