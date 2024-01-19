#pragma once

#include <string>

#include "IComponent.hpp"

namespace Resources
{
	class ShaderProgram;
	class CubeMap;
}

namespace Core::Scene::Components
{
	class NAT_API SkyBoxComponent : public IComponent
	{
	public:
		SkyBoxComponent();
		~SkyBoxComponent() override = default;

		void Render(const Maths::Mat4& mvp, const Maths::Mat4& vp, const Maths::Mat4& modelOverride, const Maths::Frustum& cameraFrustum, LowRenderer::RenderPassType pass) override;
		IComponent* CreateCopy() override;

		virtual void RenderGui() override;

		virtual ComponentType GetType() override;
		virtual void Serialize(Core::Serialization::Serializer& sr) const override;
		virtual void Deserialize(Core::Serialization::Deserializer& dr) override;
		const char* GetName() override { return "SkyBox"; }
		virtual void Delete() override;

		Resources::CubeMap* cubeMap = nullptr;
		Resources::ShaderProgram* skyShader = nullptr;
	private:

		static Resources::Mesh* boxMesh;
	};
}