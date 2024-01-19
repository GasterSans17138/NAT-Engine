#include "Core/Scene/Components/Lights/DirectionalLightComponent.hpp"

#include "Core/App.hpp"

using namespace Core::Scene::Components;
using namespace Core::Scene::Components::Lights;

DirectionalLightComponent::DirectionalLightComponent() : ILightComponent()
{
}

void DirectionalLightComponent::RenderGui()
{
	ILightComponent::RenderGui();
}

IComponent* DirectionalLightComponent::CreateCopy()
{
	return new DirectionalLightComponent(*this);
}

void DirectionalLightComponent::DataUpdate()
{
	Direction = Maths::Mat3(gameObject->transform.GetGlobal()) * Maths::Vec3(1,0,0);
	renderer->AddDirectionalLight(this);
}

ComponentType DirectionalLightComponent::GetType()
{
	return ComponentType::DirectionalLight;
}

void DirectionalLightComponent::Serialize(Core::Serialization::Serializer& sr) const
{
	ILightComponent::Serialize(sr);
}

void DirectionalLightComponent::Deserialize(Core::Serialization::Deserializer& dr)
{
	ILightComponent::Deserialize(dr);
}
