#pragma once

#include "Core/Scene/Components/IComponent.hpp"
#include "Core/Scene/GameObject.hpp"

namespace Resources
{
	class Mesh;
	class Model;
	class Material;
	class ShaderProgram;
}

namespace Core::Scene::Components
{
	class RenderModelComponent;

	namespace Game
	{
		class PortalObject;

		struct PortalObjectRef
		{
			PortalObject* portal;
			RenderModelComponent* renderer;
		};

		class NAT_API PortalGun : public IComponent
		{
		public:
			PortalGun();
			~PortalGun();

			void Init() override;
			void Update() override;

			void RenderGui() override;
			IComponent* CreateCopy() override;
			ComponentType GetType() override;
			const char* GetName() override;

			void Serialize(Core::Serialization::Serializer& sr) const override;
			void Deserialize(Core::Serialization::Deserializer& dr) override;

			void UseItem(Maths::Vec3 position, Maths::Vec3 direction, bool secondPortal);

			f32 maxDistance = 100.0f;
			Resources::Mesh* portalMesh = nullptr;
			Resources::Model* portalModel = nullptr;
			Resources::Mesh* portalMeshCollider = nullptr;
			Resources::Material* portalMaterials[2] = { nullptr, nullptr };
			Resources::Model* secondPortalModel = nullptr;
			Resources::ShaderProgram* secondPortalShader = nullptr;

		private:
			PortalObjectRef portals[2] = {};
		};
	}

}

