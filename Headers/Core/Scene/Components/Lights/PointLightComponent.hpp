#pragma once

#include "ILightComponent.hpp"

namespace Renderer
{
	class VulkanRenderer;
}

namespace Core::Scene::Components::Lights
{
	class NAT_API PointLightComponent : public ILightComponent
	{
	public:
		PointLightComponent();

		~PointLightComponent() override = default;

		void RenderGui() override;
		IComponent* CreateCopy() override;
		void DataUpdate() override;
		ComponentType GetType() override;

		void Serialize(Core::Serialization::Serializer& sr) const override;
		void Deserialize(Core::Serialization::Deserializer& dr) override;
		const char* GetName() override { return "Point Light"; }

	private:
		Maths::Vec3 Position = Maths::Vec3(0);
		f32 Quadratic = 0.12f;
		f32 Linear = 0.05f;

		friend Renderer::VulkanRenderer;
	};

}