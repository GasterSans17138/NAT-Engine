#pragma once

#include "Core/Scene/Components/IComponent.hpp"
#include "CameraComponent.hpp"
#include "Resources/Mesh.hpp"

namespace Resources
{
	class ShaderProgram;
}

namespace Renderer
{
	enum class StencilState : u8;
}

namespace Core::Scene
{
	class SceneManager;

	namespace Components::Rendering
	{
		class NAT_API PortalBaseComponent : public IComponent
		{
		public:
			PortalBaseComponent() = default;
			virtual ~PortalBaseComponent() override = default;

			void Render(const Maths::Mat4& mvp, const Maths::Mat4& vp, const Maths::Mat4& modelOverride, const Maths::Frustum& cameraFrustum, LowRenderer::RenderPassType pass) override;
			void Overwrite(const Maths::Mat4& mvp, Renderer::StencilState state, u8 value, const Maths::Vec3& color);

			virtual void DataUpdate() override;
			virtual void RenderGui() override;

			virtual void Serialize(Core::Serialization::Serializer& sr) const override;
			virtual void Deserialize(Core::Serialization::Deserializer& dr) override;

			virtual LowRenderer::Rendering::Camera UpdateCamera(const LowRenderer::Rendering::Camera& camera, Maths::Mat4& m, Maths::Vec4& nearPlane) const = 0;
			virtual Maths::Mat4 UpdateDebugBox() const = 0;
			bool IsVisibleOnScreen(const LowRenderer::Rendering::Camera* const targetCam);
			Maths::Vec4 GetScreenCovering(const Maths::Mat4& mvp);
			virtual void Delete() override;

			Maths::Vec3 boxSize = Maths::Vec3(1.0f);
			Maths::Vec3 boxOffset;
			Resources::Mesh* portalMesh = nullptr;

		private:
			static SceneManager* scenes;
			static Resources::Mesh* boxMesh;
			static Resources::ShaderProgram* portalShader;
			static Resources::ShaderProgram* depthShader;
			static Resources::ShaderProgram* boxShader;
		};
	}
}

