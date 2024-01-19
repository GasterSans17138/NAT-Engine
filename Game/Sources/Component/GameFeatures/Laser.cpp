#include "Core/App.hpp"
#include "../Game/Header/Component/GameFeatures/Laser.hpp"
#include "../Game/Header/PlayerManager.hpp"

using namespace  Core::Scene::Components::Game;
using namespace Core::Scene::Components;
using namespace Core::Serialization;

Laser::Laser()
{
	
}

Laser::~Laser()
{

}

void Laser::Init()
{
	damage = 1;
	active = true;
	soundPlayer = reinterpret_cast<Sounds::SoundPlayerComponent*>(gameObject->GetComponent(ComponentType::SoundPlayerComponent));
	SoundStopping();
}

void Laser::Update()
{
	SoundPlaying();
}



void Laser::SetDamage(f32 value)
{
	damage = value;
}

void Laser::SetActive(bool boolean)
{
	active = boolean;
}

void Laser::RenderGui()
{
	interfaceGui->DragFloat("Damage", &damage);
	interfaceGui->CheckBox("setActive", &active);
}

IComponent* Laser::CreateCopy()
{
	return new Laser(*this);
}

ComponentType Laser::GetType()
{
	return ComponentType::Laser;
}

const char* Laser::GetName()
{
	return "Laser";
}

void Laser::Serialize(Core::Serialization::Serializer& sr) const
{
	sr.Write(damage);
	sr.Write(active);
}

void Laser::Deserialize(Core::Serialization::Deserializer& dr)
{
	dr.Read(damage);
	dr.Read(active);
}

void Laser::OnCollisionStay(Colliders::ICollider* pCollider, GameObject* pOther, Colliders::ICollider* pOtherCollider, Maths::Vec3 normal)
{
	PlayerManager* player = reinterpret_cast<PlayerManager*>(pOther->GetComponent(ComponentType::PlayerManager));
	if (!player) return;
	player->life -= damage;
}

void Laser::SoundPlaying()
{
	if (!soundPlayer)
		return;
	auto app = App::GetInstance();
	if (app->GetSceneManager().GetPlayMode() == Core::Scene::PlayMode::GAME && soundPlayer->sound->path == "LaserNoise") app->GetSoundEngine().StartSound(soundPlayer->sound);
}

void Laser::SoundStopping()
{
	if (!soundPlayer)
		return;
	auto app = App::GetInstance();
	if (app->GetSceneManager().GetPlayMode() != Core::Scene::PlayMode::GAME && soundPlayer->sound->path == "LaserNoise")
	{
		app->GetSoundEngine().StopSound(soundPlayer->sound);
		app->GetSoundEngine().ResetSound(soundPlayer->sound);
	}
}
