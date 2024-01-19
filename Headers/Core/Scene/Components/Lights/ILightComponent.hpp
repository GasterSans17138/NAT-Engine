#pragma once

#include "Core/Scene/Components/IComponent.hpp"

namespace Renderer
{
	class VulkanRenderer;
}

namespace Core::Scene::Components::Lights
{
	class NAT_API ILightComponent : public IComponent
	{
	public:
		ILightComponent();

		virtual ~ILightComponent() = default;

		virtual void RenderGui() override;
		virtual void Serialize(Core::Serialization::Serializer& sr) const override;
		virtual void Deserialize(Core::Serialization::Deserializer& dr) override;

	protected:
		Maths::Vec3 AmbientColor = Maths::Vec3(0);
		Maths::Vec3 DiffuseColor = Maths::Vec3(1);
		Maths::Vec3 SpecularColor = Maths::Vec3(1);
		f32 Shininess = 16.0f;

		static Renderer::VulkanRenderer* renderer;

		friend Renderer::VulkanRenderer;
	};

}