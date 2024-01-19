#include "Core/Scene/Components/Lights/SpotLightComponent.hpp"

#include "Core/App.hpp"

using namespace Core::Scene::Components;
using namespace Core::Scene::Components::Lights;

SpotLightComponent::SpotLightComponent() : ILightComponent()
{
}

void SpotLightComponent::RenderGui()
{
	ILightComponent::RenderGui();
	interfaceGui->SliderFloat("Linear Attenuation", &Linear, 0.0f, 5.0f, false);
	interfaceGui->SliderFloat("Quadratic Attenuation", &Quadratic, 0.0f, 5.0f, false);
	interfaceGui->SliderFloat("Angle", &Angle, 0.0f, (f32)M_PI, false);
	interfaceGui->SliderFloat("Ratio", &Ratio, 0.0f, 1.0f, false);
}

IComponent* SpotLightComponent::CreateCopy()
{
	return new SpotLightComponent(*this);
}

void SpotLightComponent::DataUpdate()
{
	Position = gameObject->transform.GetGlobal().GetPositionFromTranslation();
	Direction = Maths::Mat3(gameObject->transform.GetGlobal()) * Maths::Vec3(1, 0, 0);
	renderer->AddSpotLight(this);
}

ComponentType SpotLightComponent::GetType()
{
	return ComponentType::SpotLight;
}

void SpotLightComponent::Serialize(Serialization::Serializer& sr) const
{
	ILightComponent::Serialize(sr);
	sr.Write(Linear);
	sr.Write(Quadratic);
	sr.Write(Angle);
	sr.Write(Ratio);
}

void SpotLightComponent::Deserialize(Serialization::Deserializer& dr)
{
	ILightComponent::Deserialize(dr);
	dr.Read(Linear);
	dr.Read(Quadratic);
	dr.Read(Angle);
	dr.Read(Ratio);
}
