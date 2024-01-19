#pragma once

#include "PortalBaseComponent.hpp"

namespace Core::Scene
{
	class SceneManager;

	namespace Components::Rendering
	{
		class NAT_API PortalComponent : public PortalBaseComponent
		{
		public:
			PortalComponent() = default;
			virtual ~PortalComponent() override = default;

			IComponent* CreateCopy() override;

			void RenderGui() override;

			ComponentType GetType() override;

			void Serialize(Core::Serialization::Serializer& sr) const override;
			void Deserialize(Core::Serialization::Deserializer& dr) override;
			const char* GetName() override { return "Portal"; }

			LowRenderer::Rendering::Camera UpdateCamera(const LowRenderer::Rendering::Camera& camera, Maths::Mat4& m, Maths::Vec4& nearPlane) const override;
			Maths::Mat4 UpdateDebugBox() const override;

		public:
			Maths::Vec3 targetPosition;
			Maths::Quat targetRotation;

		private:
		};
	}
}

