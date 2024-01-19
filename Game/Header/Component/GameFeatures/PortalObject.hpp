#pragma once

#include "Core/Scene/Components/IComponent.hpp"
#include "Core/Scene/GameObject.hpp"
#include "Core/Scene/Components/Rendering/PortalComponent.hpp"
#include "Core/Scene/Components/Colliders/ICollider.hpp"

namespace Core::Scene::Components::Game
{
	class PlayerManager;

	struct ObjectRef
	{
		Colliders::ICollider* collider = nullptr;
		PlayerManager* player = nullptr;
		u8 ignoreObject = 0;
	};

	class NAT_API PortalObject : public IComponent
	{
	public:
		PortalObject();
		~PortalObject();

		void Init() override;
		void Update() override;

		void RenderGui() override;
		IComponent* CreateCopy() override;
		ComponentType GetType() override;
		const char* GetName() override;

		void Serialize(Core::Serialization::Serializer& sr) const override;
		void Deserialize(Core::Serialization::Deserializer& dr) override;
		void OnCollisionBegin(Colliders::ICollider* pCollider, GameObject* pOther, Colliders::ICollider* pOtherCollider, Maths::Vec3 normal) override;
		void OnCollisionEnd(Colliders::ICollider* pCollider, GameObject* pOther, Colliders::ICollider* pOtherCollider) override;

		void OpenPortal(PortalObject* dest);
		void Attach(GameObject* parent, Maths::Vec3 offset, Maths::Quat rotOffset, bool isWall);
		void Detach();
		void ClosePortal();

		void RemoveObject(Colliders::ICollider* obj);
		void AddObject(ObjectRef obj);
		u64 GetObjectIndex(Colliders::ICollider* obj);
		Resources::Mesh* portalMesh = nullptr;

		f32 animationTime = 0.5f;

	private:
		std::vector<ObjectRef> nearObjects;
		Maths::Vec3 lastPos;
		Maths::Vec3 velocity;
		f32 animationDelay = 0.0f;
		PortalObject* destination = nullptr;
		GameObject* parentObj = nullptr;
		Maths::Vec3 offset;
		Maths::Quat rotation;
		Rendering::PortalComponent* portalRenderer = nullptr;
		bool isWall = true;
	};

}

