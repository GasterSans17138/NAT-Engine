#pragma once

#include "ILightComponent.hpp"

namespace Renderer
{
	class VulkanRenderer;
}

namespace Core::Scene::Components::Lights
{
	class NAT_API DirectionalLightComponent : public ILightComponent
	{
	public:
		DirectionalLightComponent();

		~DirectionalLightComponent() override = default;

		void RenderGui() override;
		IComponent* CreateCopy() override;
		void DataUpdate() override;
		ComponentType GetType() override;

		void Serialize(Core::Serialization::Serializer& sr) const override;
		void Deserialize(Core::Serialization::Deserializer& dr) override;
		const char* GetName() override { return "Directional Light"; }

	private:
		Maths::Vec3 Direction = Maths::Vec3(-.5f,-1,0).UnitVector();

		friend Renderer::VulkanRenderer;
	};

}