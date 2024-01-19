#include "Core/App.hpp"
#include "../Game/Header/PlayerManager.hpp"
#include "Wrappers/PhysicsEngine/JoltPhysicsEngine.hpp"

using namespace Core::Scene::Components::Game;
using namespace LowRenderer::Rendering;
using namespace Core::Serialization;

const char* const partNames[4] = {
	"RightLeg",
	"LeftLeg",
	"LeftArm",
	"RightArm"
};

namespace Core::Scene::Components::Game
{
	void PlayerManager::Init()
	{
		camera = GetCamera();
		soundPlayer = reinterpret_cast<Sounds::SoundPlayerComponent*>(gameObject->GetComponent(ComponentType::SoundPlayerComponent));
		gun = reinterpret_cast<PortalGun*>(gameObject->GetComponent(ComponentType::PortalGun));

		if (!head)
		{
			if (gameObject->childs.size() <= 0)
			{
				head = gameObject->AddChild();
				head->name = "Head";
			}
			else
			{
				head = gameObject->childs[0];
			}
			
			for (u8 i = 0; i < 4; i++)
			{
				for (auto& child : gameObject->childs)
				{
					if (child->name == partNames[i])
					{
						bodyParts[i] = child;
						break;
					}
				}
			}
		}

		if (!playerCollider)
		{
			playerCollider = reinterpret_cast<Colliders::CapsuleCollider*>(gameObject->GetComponent(ComponentType::CapsuleCollider));
			if (!playerCollider)
			{
				playerCollider = gameObject->AddComponent<Colliders::CapsuleCollider>();
			}
		}
	}
	void PlayerManager::Update()
	{
		if (SceneManager::GetInstance()->GetPlayMode() != PlayMode::GAME)
		{
			App::GetInstance()->GetPhysicsEngine().SetColliderRotation(playerCollider, Maths::Quat::AxisAngle(Maths::Vec3(0, 1, 0), Maths::Util::ToRadians(rotationY)));
			return;
		}
		if (life <= 0 && !isDead) Death();

		static Wrappers::WindowManager& window = GetWindows();

		if (!showCursor)
		{
			window.SetCursorCaptured(!window.IsCursorCaptured());
			showCursor = true;
		}

		MainThemePlaying();
		//Actions of Player
		Move();
		if (window.IsKeyPressed(GLFW_KEY_SPACE) && isGrounded)
			Jump();
		if (window.IsKeyDown(GLFW_KEY_E))
			Grab();
		else if (isGrabbing)
			UnGrab();
		if (gun)
		{
			if (window.IsMouseButtonPressed(GLFW_MOUSE_BUTTON_1))
			{
				gun->UseItem(head->transform.GetGlobal().GetPositionFromTranslation(), (head->transform.GetGlobal() * Maths::Vec4(0, 0, -1, 0)).GetVector(), false);
			}
			else if (window.IsMouseButtonPressed(GLFW_MOUSE_BUTTON_2))
			{
				gun->UseItem(head->transform.GetGlobal().GetPositionFromTranslation(), (head->transform.GetGlobal() * Maths::Vec4(0, 0, -1, 0)).GetVector(), true);
			}
		}
		if (window.IsKeyPressed(GLFW_KEY_SPACE) && canDoubleJump)
			Jump();
		if (window.IsKeyPressed(GLFW_KEY_C) && isGrounded && !isCrouched)
			Crouch();
		else if (window.IsKeyPressed(GLFW_KEY_C) && isGrounded && isCrouched)
			UnCrouch();

		if (window.IsKeyPressed(GLFW_KEY_E))
			Interaction();
	}

	Camera* PlayerManager::GetCamera()
	{
		return App::GetInstance()->GetSceneManager().GetMainCamera();
	}

	Wrappers::WindowManager& PlayerManager::GetWindows()
	{
		return App::GetInstance()->GetWindow();
	}

	IComponent* PlayerManager::CreateCopy()
	{
		return new PlayerManager(*this);
	}

	void PlayerManager::RenderGui()
	{
		interfaceGui->DragFloat("Life", &life);
		interfaceGui->DragFloat("Speed", &speed);
		interfaceGui->DragFloat("AirControl", &airControl);
		interfaceGui->DragFloat("GroundFriction", &groundFriction);
		interfaceGui->DragFloat("AirFriction", &airFriction);
		interfaceGui->DragFloat("RotationSpeed", &rotationSpeed);
		interfaceGui->DragFloat("CoefCrouch", &coefCrouch);
		interfaceGui->Separator();
		interfaceGui->SoundListCombo(&deathSound, "DeathSound");
		interfaceGui->SoundListCombo(&jumpSound, "JumpSound");
		interfaceGui->SoundListCombo(&hurtSound, "HurtSound");
	}

	ComponentType PlayerManager::GetType()
	{
		return ComponentType::PlayerManager;
	}
	void PlayerManager::OnCollisionStay(Colliders::ICollider* pCollider, GameObject* pOther, Colliders::ICollider* pOtherCollider, Maths::Vec3 normal)
	{
		if ((pOtherCollider->layerType & (LayerType::TRIGGER)))
		{
			return;
		}
		if (!isGrounded && normal.y < -0.5f)
		{
			isGrounded = true;
		}
		if ((pOtherCollider->GetComponent(ComponentType::Laser)))
		{
			if (!hurtSound) return;
			auto app = App::GetInstance();
			if (app->GetSceneManager().GetPlayMode() == Core::Scene::PlayMode::GAME && hurtSound->path == "PlayerHurt") app->GetSoundEngine().StartSound(hurtSound);
		}

	}

	void PlayerManager::OnCollisionEnd(Colliders::ICollider* pCollider, GameObject* pOther, Colliders::ICollider* pOtherCollider)
	{
		isGrounded = false;
	}

	void PlayerManager::Serialize(Serializer& sr) const
	{
		sr.Write(life);
		sr.Write(speed);
		sr.Write(airControl);
		sr.Write(groundFriction);
		sr.Write(airFriction);
		sr.Write(isDead);
		bool canDoubleJump = false;
		bool isJumping = false;
		bool isGrounded = false;
		bool isCrouched = false;
		bool isGrabbing = false;
		u8 mask = (u8)(isJumping) | (u8)(canDoubleJump << 1) | (u8)(isGrounded << 2) | (u8)(isCrouched << 3) | (u8)(isGrabbing << 4);
		sr.Write(mask);
		sr.Write(deathSound ? deathSound->hash : (u64)0);
		sr.Write(jumpSound ? jumpSound->hash : (u64)0);
		sr.Write(hurtSound ? hurtSound->hash : (u64)0);
	}
	void PlayerManager::Deserialize(Deserializer& dr)
	{
		dr.Read(life);
		dr.Read(speed);
		dr.Read(airControl);
		dr.Read(groundFriction);
		dr.Read(airFriction);
		dr.Read(isDead);
		u8 mask = 0;
		dr.Read(mask);
		isJumping = mask & 0x1;
		canDoubleJump = mask & 0x2;
		isGrounded = mask & 0x4;
		isCrouched = mask & 0x8;
		isGrabbing = mask & 0x10;
		u64 hash;
		dr.Read(hash);
		auto& res = App::GetInstance()->GetResources();
		deathSound = res.Get<Resources::Sound>(hash);
		dr.Read(hash);
		auto& res2 = App::GetInstance()->GetResources();
		jumpSound = res2.Get<Resources::Sound>(hash);
		dr.Read(hash);
		auto& res3 = App::GetInstance()->GetResources();
		hurtSound = res3.Get<Resources::Sound>(hash);
	}
	const char* PlayerManager::GetName()
	{
		return "PlayerManager";
	}
	void PlayerManager::MainThemePlaying()
	{
		if (!soundPlayer)
			return;
		auto app = App::GetInstance();
		if (app->GetSceneManager().GetPlayMode() == Core::Scene::PlayMode::GAME && soundPlayer->sound->path == "MainTheme") app->GetSoundEngine().StartSound(soundPlayer->sound);
	}
	void PlayerManager::Move()
	{
		Wrappers::WindowManager& window = GetWindows();
		float dSpeed = window.GetDeltaTime() * speed * (window.IsKeyDown(GLFW_KEY_LEFT_SHIFT) ? 1.3f : (window.IsKeyDown(GLFW_KEY_LEFT_CONTROL) ? 0.2f : 1.0f));

		Maths::Vec2 delta = Maths::Vec2(dSpeed * window.IsKeyDown(GLFW_KEY_D) - dSpeed * window.IsKeyDown(GLFW_KEY_A), dSpeed * window.IsKeyDown(GLFW_KEY_S) - dSpeed * window.IsKeyDown(GLFW_KEY_W));

		rotationX = Maths::Util::Clamp(rotationX - window.GetCursorDelta().y * rotationSpeed, -90.0f, 90.0f);
		rotationY = Maths::Util::Mod(rotationY - window.GetCursorDelta().x * rotationSpeed, 360.0f);

		head->transform.SetRotation(Maths::Quat::AxisAngle(Maths::Vec3(1, 0, 0), Maths::Util::ToRadians(rotationX)));
		Maths::Quat targetRotation = Maths::Quat::AxisAngle(Maths::Vec3(0, 1, 0), Maths::Util::ToRadians(rotationY));

		f32 angle = static_cast<f32>(sin(window.GetWindowTime() * 10.0)) * (f32)(M_PI_4) * (delta.GetLength() / dSpeed);
		for (u8 i = 0; i < 4; ++i)
		{
			if (!bodyParts[i]) continue;
			bodyParts[i]->transform.SetRotation(Maths::Quat::AxisAngle(Maths::Vec3(1, 0, 0), i & 1 ? angle : -angle));
		}

		auto& physics = App::GetInstance()->GetPhysicsEngine();
		Maths::Vec3 pos = gameObject->transform.GetPosition();
		Maths::Vec3 vel = physics.GetBodyVelocity(playerCollider);
		vel *= isGrounded ? Maths::Vec3(0.5f, 1.0f, 0.5f) : Maths::Vec3(0.98f, 1.0f, 0.98f);
		if (!isGrounded) delta *= 0.02f;
		pos += (gameObject->transform.GetGlobal() * Maths::Vec4(1, 0, 0, 0)).GetVector() * delta.x;
		pos += (gameObject->transform.GetGlobal() * Maths::Vec4(0, 0, 1, 0)).GetVector() * delta.y;
		pos += vel * window.GetDeltaTime();
		physics.SetFriction(playerCollider, 0.0f);
		physics.MoveColliderTo(playerCollider, pos + playerCollider->GetPositionOffset(), targetRotation, window.GetDeltaTime());
		head->transform.Update();
		Maths::Mat4 mat = head->transform.GetGlobal();
		camera->position = mat.GetPositionFromTranslation();
		camera->focus = (mat * Maths::Vec4(0, 0, -1, 0)).GetVector().UnitVector() + camera->position;
		camera->up = (mat * Maths::Vec4(0, 1, 0, 0)).GetVector().UnitVector();
	}
	void PlayerManager::Jump()
	{
		Colliders::CapsuleCollider* collider = reinterpret_cast<Colliders::CapsuleCollider*>(gameObject->GetComponent(ComponentType::CapsuleCollider));
		App::GetInstance()->GetPhysicsEngine().SetBodyVelocity(collider, Maths::Vec3(collider->GetVelocity().x, 8, collider->GetVelocity().z));
		isGrounded = false;
		canDoubleJump = false;
		auto app = App::GetInstance();
		if (!jumpSound) return;
		if (app->GetSceneManager().GetPlayMode() == Core::Scene::PlayMode::GAME && jumpSound->path == "Jump") app->GetSoundEngine().StartSound(jumpSound);
	}
	void PlayerManager::Grab()
	{
		auto& physics = App::GetInstance()->GetPhysicsEngine();
		if (!isGrabbing)
		{
			if (!physics.RayCast(camera->position, (camera->focus - camera->position).UnitVector() * 10, result, Core::Scene::Components::LayerType::OBJECT))
			{
				return;
			}
			isGrabbing = true;
			result.hitCollider->SetLayerType(LayerType::ALLBUTPLAYER);
		}
		Maths::Vec3 pos = result.hitCollider->gameObject->transform.GetPosition();
		Maths::Mat4 mat = head->transform.GetGlobal();
		Maths::Vec3 forward = (mat * Maths::Vec4(0, 0, -1, 0)).GetVector();
		forward = forward.UnitVector();
		Maths::Vec3 pos2 = forward * 6 + camera->position;
		if ((pos - pos2).GetLength() > 3.0f)
		{
			UnGrab();
			return;
		}
		physics.MoveColliderTo(result.hitCollider, pos2, head->transform.GetGlobal(), GetWindows().GetDeltaTime());

	}
	void PlayerManager::UnGrab()
	{
		isGrabbing = false;
		result.hitCollider->SetLayerType(static_cast<LayerType>(LayerType::GROUND | LayerType::WALL | LayerType::CEILING | LayerType::OBJECT | LayerType::ALLBUTPLAYER));
	}

	void PlayerManager::Interaction()
	{
		auto& physics = App::GetInstance()->GetPhysicsEngine();
		Wrappers::RayCastResult clickedButton;
		if (physics.RayCast(camera->position, (camera->focus - camera->position).UnitVector() * 10, clickedButton, Core::Scene::Components::LayerType::BUTTON))
		{
			Button* button = reinterpret_cast<Button*>(clickedButton.hitCollider->gameObject->GetComponent(Core::Scene::Components::ComponentType::Button));
			if (button)
				button->OnClick();
			ButtonDoor* buttondoor = reinterpret_cast<ButtonDoor*>(clickedButton.hitCollider->gameObject->GetComponent(Core::Scene::Components::ComponentType::ButtonDoor));
			if (buttondoor)
				buttondoor->OnClick();
			ButtonPortal* buttonPortal = reinterpret_cast<ButtonPortal*>(clickedButton.hitCollider->gameObject->GetComponent(Core::Scene::Components::ComponentType::ButtonPortal));
			if (buttonPortal)
				buttonPortal->OnClick();
		}
	}

	void PlayerManager::Crouch()
	{
		speed /= coefCrouch;
		isCrouched = true;
		//Shrink size player by 2
		App::GetInstance()->GetPhysicsEngine().SetCapsuleHeight(playerCollider, 0.5f);
	}
	void PlayerManager::UnCrouch()
	{
		speed *= coefCrouch;
		isCrouched = false;
		App::GetInstance()->GetPhysicsEngine().SetCapsuleHeight(playerCollider, 2.7f);
	}
	void PlayerManager::Death()
	{
		isDead = true;
		if (!deathSound) return;
		auto app = App::GetInstance();
		if (app->GetSceneManager().GetPlayMode() == Core::Scene::PlayMode::GAME && deathSound->path == "Death") app->GetSoundEngine().StartSound(deathSound);
		app->GetSceneManager().idScenetoLoad = 10;
		app->GetSceneManager().ifSwapScene = true;
	}
}