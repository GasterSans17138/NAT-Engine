#include "Core/Scene/Components/Lights/ILightComponent.hpp"

#include "Core/App.hpp"

using namespace Core::Scene::Components::Lights;

Renderer::VulkanRenderer* ILightComponent::renderer = nullptr;

ILightComponent::ILightComponent() : IComponent()
{
	renderer = &Core::App::GetInstance()->GetRenderer();
}

void ILightComponent::RenderGui()
{
	interfaceGui->ColorEdit3("Ambient Light", AmbientColor);
	interfaceGui->ColorEdit3("Diffuse Light", DiffuseColor);
	interfaceGui->ColorEdit3("Specular Light", SpecularColor);
	interfaceGui->SliderFloat("Shininess", &Shininess, 0.01f, 65536.0f, true);
}

void ILightComponent::Serialize(Core::Serialization::Serializer& sr) const
{
	sr.Write(AmbientColor);
	sr.Write(DiffuseColor);
	sr.Write(SpecularColor);
	sr.Write(Shininess);
}

void ILightComponent::Deserialize(Core::Serialization::Deserializer& dr)
{
	dr.Read(AmbientColor);
	dr.Read(DiffuseColor);
	dr.Read(SpecularColor);
	dr.Read(Shininess);
}
