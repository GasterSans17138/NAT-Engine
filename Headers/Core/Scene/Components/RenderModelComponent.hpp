#pragma once

#include <string>

#include "IComponent.hpp"

#include "Resources/Mesh.hpp"
#include "Resources/ShaderProgram.hpp"
#include "Resources/Material.hpp"
#include "Resources/Model.hpp"

namespace Core::Scene::Components
{
	class NAT_API RenderModelComponent : public IComponent
	{
	public:
		RenderModelComponent();
		~RenderModelComponent() override = default;

		void UseModel(Resources::Model* pModel);

		void Render(const Maths::Mat4& mvp, const Maths::Mat4& vp, const Maths::Mat4& modelOverride, const Maths::Frustum& cameraFrustum, LowRenderer::RenderPassType pass) override;
		IComponent* CreateCopy() override;

		virtual void RenderGui() override;

		virtual ComponentType GetType() override;
		virtual void Serialize(Core::Serialization::Serializer& sr) const override;
		virtual void Deserialize(Core::Serialization::Deserializer& dr) override;
		const char* GetName() override { return "Static Model"; }

		bool HasMeshes() const {return meshes.size() != 0; };
		virtual void Delete() override;

		Resources::ShaderProgram* shader = nullptr;
		std::vector<Resources::Material*> materials;
		std::vector<Resources::Mesh*> meshes;
		LowRenderer::RenderPassType targetPass = LowRenderer::RenderPassType::ALL;
		bool useCulling = true;
		bool hideSecond = false;
		bool hideFirst = false;
	private:

		///Reference to the model used by the object, can be null.
		Resources::Model* mUsedModel = nullptr;
	};
}