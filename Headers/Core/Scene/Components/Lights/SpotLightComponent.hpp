#pragma once

#include "ILightComponent.hpp"

namespace Renderer
{
	class VulkanRenderer;
}

namespace Core::Scene::Components::Lights
{
	class NAT_API SpotLightComponent : public ILightComponent
	{
	public:
		SpotLightComponent();

		~SpotLightComponent() override = default;

		void RenderGui() override;
		IComponent* CreateCopy() override;
		void DataUpdate() override;
		ComponentType GetType() override;

		void Serialize(Core::Serialization::Serializer& sr) const override;
		void Deserialize(Core::Serialization::Deserializer& dr) override;
		const char* GetName() override { return "Spot Light"; }

	private:
		Maths::Vec3 Position = Maths::Vec3(0);
		Maths::Vec3 Direction = Maths::Vec3(1,0,0);
		f32 Quadratic = 0.12f;
		f32 Linear = 0.05f;
		f32 Angle = 1.0f; // Angle is in radians
		f32 Ratio = 0.05f; // Ratio between inner angle / outer angle : ratio of 0 is inner = outer

		friend Renderer::VulkanRenderer;
	};

}