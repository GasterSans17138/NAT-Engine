#include "Core/App.hpp"
#include "../Game/Header/Component/GameFeatures/Bumper.hpp"
#include "Wrappers/PhysicsEngine/JoltPhysicsEngine.hpp"
#include "../Game/Header/PlayerManager.hpp"


using namespace  Core::Scene::Components::Game;
using namespace Core::Scene::Components;
using namespace Core::Serialization;

Bumper::Bumper()
{
}

Bumper::~Bumper()
{
}

void Bumper::GetCollider()
{
	collider = reinterpret_cast<Colliders::CubeCollider*>(GetComponent(ComponentType::CubeCollider));
}

void Bumper::SoundPlaying()
{
	if (!soundPlayer)
		return;
	auto app = App::GetInstance();
	if (app->GetSceneManager().GetPlayMode() == Core::Scene::PlayMode::GAME && soundPlayer->sound->path == "Bumper") app->GetSoundEngine().StartSound(soundPlayer->sound);
}

void Bumper::Init()
{
	GetCollider();
	soundPlayer = reinterpret_cast<Sounds::SoundPlayerComponent*>(gameObject->GetComponent(ComponentType::SoundPlayerComponent));
}

void Bumper::Update()
{
	
}

void Bumper::OnCollisionBegin(Colliders::ICollider* pCollider, GameObject* pOther, Colliders::ICollider* pOtherCollider, Maths::Vec3 normal)
{
		if (naturalBumper)
			App::GetInstance()->GetPhysicsEngine().AddForce(*pOtherCollider, normal* force);
		else
			App::GetInstance()->GetPhysicsEngine().AddForce(*pOtherCollider, dir * force);
		SoundPlaying();
}	

Core::Scene::Components::IComponent* Bumper::CreateCopy()
{
	return new Bumper(*this);
}

void Bumper::RenderGui()
{
	interfaceGui->CheckBox("NaturalBumper", &naturalBumper);
	if(!naturalBumper)
		interfaceGui->DragFloat3("Direction", &dir.x);
	interfaceGui->DragFloat("Force", &force);
}

ComponentType Bumper::GetType()
{
	return ComponentType::Bumper;
}

void Bumper::Serialize(Serializer& sr) const
{
	sr.Write(naturalBumper);
	sr.Write(force);
	sr.Write(dir);
	//sr.Write(collider);
}

void Bumper::Deserialize(Deserializer& dr)
{
	dr.Read(naturalBumper);
	dr.Read(force);
	dr.Read(dir);
	//dr.Read(collider);
}

const char* Bumper::GetName()
{
	return "Bumper";
}