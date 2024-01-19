#include "Core/App.hpp"
#include "../Game/Header/Component/GameFeatures/DoubleJump.hpp"
#include "../Game/Header/PlayerManager.hpp"

using namespace  Core::Scene::Components::Game;
using namespace Core::Scene::Components;
using namespace Core::Serialization;

DoubleJump::DoubleJump()
{
}

DoubleJump::~DoubleJump()
{
}

void DoubleJump::Init()
{
	if (gameObject->childs.size() == 0)
	{
		gameObject->AddChild();
		childTexture = gameObject->childs[0];
	}
	else
	{
		childTexture = gameObject->childs[0];
	}
	soundPlayer = reinterpret_cast<Sounds::SoundPlayerComponent*>(gameObject->GetComponent(ComponentType::SoundPlayerComponent));
}

void DoubleJump::Update()
{
	if (gameObject->childs.size() == 0)
		childTexture = gameObject->childs[0];
	if (timeBeforeRespawn > 0)
	{
		timeBeforeRespawn -= App::GetInstance()->GetWindow().GetDeltaTime();
		if (timeBeforeRespawn <= 0)
		{
			childTexture->SetActive(true);
		}
	}
}

void DoubleJump::OnCollisionBegin(Colliders::ICollider* pCollider, GameObject* pOther, Colliders::ICollider* pOtherCollider, Maths::Vec3 normal)
{
	LOG_DEFAULT("doubleJump");
	PlayerManager* player = reinterpret_cast<PlayerManager*>(pOther->GetComponent(ComponentType::PlayerManager));
	if (!player) return;
	if (timeBeforeRespawn <= 0)
		player->canDoubleJump = true;
	if(childTexture->IsActive())
		SoundPlaying();
	childTexture->SetActive(false);
	timeBeforeRespawn = respawnTime;
}

void DoubleJump::SoundPlaying()
{
	if (!soundPlayer)
		return;
	auto app = App::GetInstance();
	if (app->GetSceneManager().GetPlayMode() == Core::Scene::PlayMode::GAME && soundPlayer->sound->path == "DoubleJump") app->GetSoundEngine().StartSound(soundPlayer->sound);
}

IComponent* DoubleJump::CreateCopy()
{
	return new DoubleJump(*this);
}

void DoubleJump::RenderGui()
{
	interfaceGui->DragFloat("Respawn time", &respawnTime);
}

ComponentType DoubleJump::GetType()
{
	return ComponentType::DoubleJump;
}

void DoubleJump::Serialize(Serializer& sr) const
{
	sr.Write(respawnTime);
}

void DoubleJump::Deserialize(Deserializer& dr)
{
	dr.Read(respawnTime);
}

const char* DoubleJump::GetName()
{
	return "DoubleJump";
}