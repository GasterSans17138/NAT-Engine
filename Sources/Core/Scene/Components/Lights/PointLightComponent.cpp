#include "Core/Scene/Components/Lights/PointLightComponent.hpp"

#include "Core/App.hpp"

using namespace Core::Scene::Components;
using namespace Core::Scene::Components::Lights;

PointLightComponent::PointLightComponent() : ILightComponent()
{
}

void PointLightComponent::RenderGui()
{
	ILightComponent::RenderGui();
	interfaceGui->SliderFloat("Linear Attenuation", &Linear, 0.0f, 2.0f, false);
	interfaceGui->SliderFloat("Quadratic Attenuation", &Quadratic, 0.0f, 2.0f, false);
}

IComponent* PointLightComponent::CreateCopy()
{
	return new PointLightComponent(*this);
}

void PointLightComponent::DataUpdate()
{
	Position = gameObject->transform.GetGlobal().GetPositionFromTranslation();
	renderer->AddPointLight(this);
}

ComponentType PointLightComponent::GetType()
{
	return ComponentType::PointLight;
}

void PointLightComponent::Serialize(Serialization::Serializer& sr) const
{
	ILightComponent::Serialize(sr);
	sr.Write(Linear);
	sr.Write(Quadratic);
}

void PointLightComponent::Deserialize(Serialization::Deserializer& dr)
{
	ILightComponent::Deserialize(dr);
	dr.Read(Linear);
	dr.Read(Quadratic);
}
