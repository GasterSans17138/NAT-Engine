#include "Core/App.hpp"
#include "../Game/Header/Component/GameFeatures/ButtonPortal.hpp"



using namespace Core::Scene::Components::Game;
using namespace Core::Scene::Components;
using namespace LowRenderer::Rendering;
using namespace Core::Serialization;



ButtonPortal::ButtonPortal()
{
}

ButtonPortal::~ButtonPortal()
{
}


void ButtonPortal::Init()
{
	if (gameObject->childs.size() >= 2)
	{
		surfacec1 = gameObject->childs[0];
		surfacec2 = gameObject->childs[1];
	}
}

void ButtonPortal::Update()
{

}


void ButtonPortal::RenderGui()
{
	//if (!Surfacec1 || !Surfacec2)

	interfaceGui->Text("This Button need to have 2 surface child with collider with Portal Layer");
}

IComponent* ButtonPortal::CreateCopy()
{
	return new ButtonPortal(*this);
}

ComponentType ButtonPortal::GetType()
{
	return ComponentType::ButtonPortal;
}

const char* ButtonPortal::GetName()
{
	return "ButtonPortal";
}

void ButtonPortal::OnClick()
{
	surfacec1->AddComponent<Rendering::PortalComponent>();
	surfacec2->AddComponent<Rendering::PortalComponent>();
}

void ButtonPortal::SoundPlaying()
{
	auto app = App::GetInstance();
	if (app->GetSceneManager().GetPlayMode() == Core::Scene::PlayMode::GAME && soundPlayer->sound->path == "Button") app->GetSoundEngine().StartSound(soundPlayer->sound);
}

void ButtonPortal::Serialize(Core::Serialization::Serializer& sr) const
{

}

void ButtonPortal::Deserialize(Core::Serialization::Deserializer& dr)
{

}