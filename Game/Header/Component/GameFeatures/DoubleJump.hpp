#pragma once
#include "Core/Scene/SceneManager.hpp"
#include "Core/Scene/GameObject.hpp"

namespace Core::Scene::Components::Game
{
	class NAT_API DoubleJump : public IComponent
	{
	public:
		DoubleJump();
		~DoubleJump() override;

		void Init() override;
		void Update() override;

		IComponent* CreateCopy() override;
		virtual void RenderGui() override;
		virtual ComponentType GetType() override;
		virtual void Serialize(Core::Serialization::Serializer& sr) const override;
		virtual void Deserialize(Core::Serialization::Deserializer& dr) override;
		const char* GetName() override;

		void OnCollisionBegin(Colliders::ICollider* pCollider, GameObject* pOther, Colliders::ICollider* pOtherCollider, Maths::Vec3 normal) override;
		void SoundPlaying();

	private:
		f32 respawnTime = 5.0f;
		f32 timeBeforeRespawn = 0;
		GameObject* childTexture;
		Core::Scene::Components::Sounds::SoundPlayerComponent* soundPlayer = nullptr;
	};
}



