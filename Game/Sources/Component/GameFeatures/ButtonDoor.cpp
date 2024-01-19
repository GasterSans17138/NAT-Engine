#include "Core/App.hpp"
#include "../Game/Header/Component/GameFeatures/ButtonDoor.hpp"



using namespace Core::Scene::Components::Game;
using namespace Core::Scene::Components;
using namespace LowRenderer::Rendering;
using namespace Core::Serialization;



ButtonDoor::ButtonDoor()
{
}

ButtonDoor::~ButtonDoor()
{
}


void ButtonDoor::Init()
{
	if (gameObject->childs.size())
		door = gameObject->childs[0];
	soundButton = reinterpret_cast<Sounds::SoundPlayerComponent*>(gameObject->GetComponent(ComponentType::SoundPlayerComponent));
}

void ButtonDoor::Update()
{
	if (door && moveTimer > 0.0f)
	{
		moveTimer = Maths::Util::MaxF(moveTimer - App::GetInstance()->GetWindow().GetDeltaTime(), 0.0f);
		if (openDoor)
		{
			door->transform.SetPosition(Maths::Util::Lerp(coordEnd, coordBegin, moveTimer / closeTime));
		}
		else
		{
			door->transform.SetPosition(Maths::Util::Lerp(coordBegin, coordEnd, moveTimer / closeTime));
		}
	}
	if (door && isTrigger)
	{
		moveTimer = Maths::Util::MaxF(moveTimer - App::GetInstance()->GetWindow().GetDeltaTime(), 0.0f);
		if (openDoor)
		{
			door->transform.SetPosition(Maths::Util::Lerp(coordEnd, coordBegin, moveTimer / closeTime));
		}
		else
		{
			door->transform.SetPosition(Maths::Util::Lerp(coordBegin, coordEnd, moveTimer / closeTime));
		}
	}
}


void ButtonDoor::RenderGui()
{
	interfaceGui->CheckBox("is Trigger", &isTrigger);
	interfaceGui->SliderFloat3("Begin Coord", &coordBegin.x, 0, 100);
	interfaceGui->SliderFloat3("End Coord", &coordEnd.x, 0, 100);
	if (interfaceGui->Button("Syncronize"))
	{
		if (gameObject->childs.size())
			door = gameObject->childs[0];
		if (door) 
		{
			coordBegin = door->transform.GetPosition();
			coordEnd = door->transform.GetPosition();
		}
	}
}

IComponent* ButtonDoor::CreateCopy()
{
	return new ButtonDoor(*this);
}

ComponentType ButtonDoor::GetType()
{
	return ComponentType::ButtonDoor;
}

const char* ButtonDoor::GetName()
{
	return "ButtonDoor";
}

void ButtonDoor::OnClick()
{
	if (door && !isTrigger && moveTimer <= 0)	
	{
		openDoor = !openDoor;
		moveTimer = closeTime;
		SoundPlaying();
	}
}

void ButtonDoor::SoundPlaying()
{
	if (!soundButton)
		return;
	auto app = App::GetInstance();
	if (app->GetSceneManager().GetPlayMode() == Core::Scene::PlayMode::GAME && soundButton->sound->path == "Button") app->GetSoundEngine().StartSound(soundButton->sound);
}

void ButtonDoor::OnCollisionStay(Colliders::ICollider* pCollider, GameObject* pOther, Colliders::ICollider* pOtherCollider, Maths::Vec3 normal)
{
	if (door && isTrigger && door->transform.GetPosition() == coordBegin)
	{
		moveTimer = closeTime;
		openDoor = true;
	}
}

void ButtonDoor::OnCollisionEnd(Colliders::ICollider* pCollider, GameObject* pOther, Colliders::ICollider* pOtherCollider)
{
	if (door && isTrigger)
	{
		moveTimer = closeTime;
		openDoor = false;
	}
}

void ButtonDoor::Serialize(Core::Serialization::Serializer& sr) const
{
	sr.Write(isTrigger);
	sr.Write(coordBegin);
	sr.Write(coordEnd);
}

void ButtonDoor::Deserialize(Core::Serialization::Deserializer& dr)
{
	dr.Read(isTrigger);
	dr.Read(coordBegin);
	dr.Read(coordEnd);
}