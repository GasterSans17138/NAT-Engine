#pragma once

#include "LowRenderer/FrameBuffer.hpp"
#include "LowRenderer/Rendering/Camera.hpp"
#include "Maths/Maths.hpp"

#include "Core/Scene/Components/IComponent.hpp"

namespace Resources
{
	class ShaderProgram;
}

namespace Core::Scene
{
	class SceneManager;

	namespace Components::Rendering
	{
		class NAT_API CameraComponent : public IComponent
		{
		public:
			CameraComponent();
			CameraComponent(const CameraComponent&);
			~CameraComponent() = default;

			IComponent* CreateCopy() override;
			void DataUpdate() override;
			void Render(const Maths::Mat4& mvp, const Maths::Mat4& vp, const Maths::Mat4& modelOverride, const Maths::Frustum& cameraFrustum, LowRenderer::RenderPassType pass) override;

			Maths::Mat4 GetVPMatrix() const;

			void RenderGui() override;

			ComponentType GetType() override;

			void Serialize(Core::Serialization::Serializer& sr) const override;
			void Deserialize(Core::Serialization::Deserializer& dr) override;
			const char* GetName() override { return "Render Camera"; }
			virtual void Delete() override;

		public:
			LowRenderer::FrameBuffer* frameBuffer = nullptr;
			LowRenderer::Rendering::Camera camera;

		private:
			Maths::IVec2 tmpResolution;
			static SceneManager* scenes;
			static Resources::ShaderProgram* boxShader;
		};
	}
}

