#pragma once
#include "../Game/Header/Component/GameFeatures/Button.hpp"



namespace Core::Scene::Components::Game
{

	class NAT_API ButtonDoor : public Button
	{
	public:
		ButtonDoor();
		~ButtonDoor();

		void Init() override;
		void Update() override;
		IComponent* CreateCopy() override;
		void RenderGui() override;
		ComponentType GetType() override;

		void Serialize(Core::Serialization::Serializer& sr) const override;
		void Deserialize(Core::Serialization::Deserializer& dr) override;
		const char* GetName() override;

		void OnClick() override;
		void OnCollisionStay(Colliders::ICollider* pCollider, GameObject* pOther, Colliders::ICollider* pOtherCollider, Maths::Vec3 normal) override;
		void OnCollisionEnd(Colliders::ICollider* pCollider, GameObject* pOther, Colliders::ICollider* pOtherCollider) override;
		void SoundPlaying() override;

	private:
		GameObject* door;
		bool openDoor = false;
		bool isTrigger = false;
		Maths::Vec3 coordBegin = { 0,0,0 };
		Maths::Vec3 coordEnd = { 0,0,0 };
		f32 closeTime = 2.0f;
		f32 moveTimer = 0.0f;
		Core::Scene::Components::Sounds::SoundPlayerComponent* soundButton = nullptr;
	};
}

