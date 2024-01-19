#pragma once
#include "Core/Scene/SceneManager.hpp"
#include "Core/Scene/GameObject.hpp"
#include "Core/Scene/Components/Colliders/CubeCollider.hpp"

namespace Core::Scene::Components::Game
{
	class NAT_API Bumper : public IComponent
	{
	public:
		Bumper();
		~Bumper();

		void Init() override;
		void Update() override;

		IComponent* CreateCopy() override;
		virtual void RenderGui() override;
		virtual ComponentType GetType() override;
		virtual void Serialize(Core::Serialization::Serializer& sr) const override;
		virtual void Deserialize(Core::Serialization::Deserializer& dr) override;
		const char* GetName() override;

		void OnCollisionBegin(Colliders::ICollider* pCollider, GameObject* pOther, Colliders::ICollider* pOtherCollider, Maths::Vec3 normal) override;

		void GetCollider();
		void SoundPlaying();

	private:
		bool naturalBumper = true;
		f32 force = 1;
		Maths::Vec3 dir = {0,0,0};
		Colliders::CubeCollider* collider = nullptr;
		Core::Scene::Components::Sounds::SoundPlayerComponent* soundPlayer = nullptr;
	};
}