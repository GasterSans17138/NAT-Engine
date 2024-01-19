#pragma once

#include "Core/Scene/Components/IComponent.hpp"
#include "Core/Scene/GameObject.hpp"

namespace Core::Scene::Components::Game
{
	class NAT_API Laser : public IComponent
	{
	public:
		Laser();
		~Laser();

		void Init() override;
		void Update() override;
		void SetDamage(f32 value);
		void SetActive(bool boolean);

		virtual IComponent* CreateCopy();
		void RenderGui() override;
		virtual ComponentType GetType();
		virtual const char* GetName();

		virtual void Serialize(Core::Serialization::Serializer& sr) const override;
		virtual void Deserialize(Core::Serialization::Deserializer& dr) override;

		void OnCollisionStay(Colliders::ICollider* pCollider, GameObject* pOther, Colliders::ICollider* pOtherCollider, Maths::Vec3 normal) override;
		void SoundPlaying();
		void SoundStopping();

	private:
		f32 damage;
		bool active;
		Core::Scene::Components::Sounds::SoundPlayerComponent* soundPlayer = nullptr;
	};

}

