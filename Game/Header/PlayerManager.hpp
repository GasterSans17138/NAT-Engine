#pragma once
#include "Core/Scene/GameObject.hpp"
#include "Core/Scene/Components/Colliders/ICollider.hpp"
#include "Core/Scene/Components/Colliders/CubeCollider.hpp"
#include "../Game/Header/Component/GameFeatures/PortalGun.hpp"
#include "Core/Scene/SceneManager.hpp"

namespace Core::Scene::Components::Game
{
	class NAT_API PlayerManager : public IComponent
	{
	public:
		PlayerManager() = default;
		~PlayerManager() = default;

		LowRenderer::Rendering::Camera* GetCamera();
		Wrappers::WindowManager& GetWindows();

		void Init() override;
		void Update() override;
		IComponent* CreateCopy() override;
		virtual void RenderGui() override;
		virtual ComponentType GetType() override;
		void OnCollisionStay(Colliders::ICollider* pCollider, GameObject* pOther, Colliders::ICollider* pOtherCollider, Maths::Vec3 normal) override;
		void OnCollisionEnd(Colliders::ICollider* pCollider, GameObject* pOther, Colliders::ICollider* pOtherCollider) override;
		

		virtual void Serialize(Core::Serialization::Serializer& sr) const override;
		virtual void Deserialize(Core::Serialization::Deserializer& dr) override;
		const char* GetName() override;
		void MainThemePlaying();

		GameObject* head = nullptr;
		GameObject* bodyParts[4] = { nullptr };
		Colliders::CapsuleCollider* playerCollider = nullptr;
		LowRenderer::Rendering::Camera* camera = nullptr;
		Wrappers::RayCastResult result;
		f32 rotationX = 0.f;
		f32 rotationY = 0.f;
		f32 life = 5.f;
		f32 speed = 5.f;
		f32 rotationSpeed = 0.2f;
		f32 coefCrouch = 2.f;
		f32 airControl = 0.02f;
		f32 groundFriction = 0.5f;
		f32 airFriction = 0.98f;
		bool showCursor = false;
		bool canDoubleJump = false;
		bool isJumping = false;
		bool isGrounded = false;
		bool isCrouched = false;
		bool isGrabbing = false;
		bool isDead = false;
	private:
		Sounds::SoundPlayerComponent* soundPlayer = nullptr;
		PortalGun* gun = nullptr;
		Resources::Sound* deathSound = nullptr;
		Resources::Sound* jumpSound = nullptr;
		Resources::Sound* hurtSound = nullptr;
		void Move();
		void Crouch();
		void UnCrouch();
		void Grab();
		void UnGrab();
		void Interaction();
		void Jump();
		void Death();

	};

}