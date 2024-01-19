#pragma once
#include "Core/Scene/Components/IComponent.hpp"

namespace Core::Scene::Components::Game
{

	class NAT_API Button : public IComponent
	{
	public:
		Button();
		virtual ~Button() = default;

		void Init() override;
		void Update() override;
		IComponent* CreateCopy() override;
		virtual void RenderGui() override;
		virtual ComponentType GetType() override;

		virtual void Serialize(Core::Serialization::Serializer& sr) const override;
		virtual void Deserialize(Core::Serialization::Deserializer& dr) override;
		virtual void OnCollisionBegin(Colliders::ICollider* pCollider, GameObject* pOther, Colliders::ICollider* pOtherCollider, Maths::Vec3 normal) override;
		const char* GetName() override;

		virtual void OnClick();
		virtual void SoundPlaying();
	private:
	};

}

