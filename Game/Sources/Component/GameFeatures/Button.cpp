#include "Core/App.hpp"
#include "../Game/Header/Component/GameFeatures/Button.hpp"



using namespace Core::Scene::Components::Game;
using namespace Core::Scene::Components;
using namespace LowRenderer::Rendering;
using namespace Core::Serialization;



Button::Button()
{
}


void Button::Init()
{
	
	
}

void Button::Update()
{
	
}


void Button::RenderGui()
{

}

IComponent* Button::CreateCopy()
{
	return new Button(*this);
}

ComponentType Button::GetType()
{
	return ComponentType::Button;
}

const char* Button::GetName()
{
	return "Button";
}

void Button::OnCollisionBegin(Colliders::ICollider* pCollider, GameObject* pOther, Colliders::ICollider* pOtherCollider, Maths::Vec3 normal)
{

}

void Button::OnClick()
{
	
}

void Button::SoundPlaying()
{
}


void Button::Serialize(Core::Serialization::Serializer& sr) const
{

}

void Button::Deserialize(Core::Serialization::Deserializer& dr)
{

}