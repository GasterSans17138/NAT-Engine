#pragma once

#include "Resources/ResourceManager.hpp"

#include "Core/Scene/Components/Colliders/ICollider.hpp"

namespace Resources
{
	class Model;
	class Mesh;
	class IResource;
}

namespace Wrappers::PhysicsEngine
{
	class JoltPhysicsEngine;
}

namespace Core::Scene::Components::Colliders
{
	class NAT_API MeshCollider : public ICollider
	{
	public:
		MeshCollider()  = default;
		~MeshCollider() = default;

		void Serialize(Core::Serialization::Serializer& sr) const;
		void Deserialize(Core::Serialization::Deserializer& dr);
		void Init() override;

		IComponent* CreateCopy() override;
		void RenderGui() override;
		ComponentType GetType() override;
		const char* GetName() override;

		void UseModel(const Resources::Model* pModel);
		void UseMesh(const Resources::Mesh* pMesh);
		virtual void Delete() override;
		
		void OnScaleUpdate(const Maths::Vec3& pSize) override;

	private:
		Resources::ObjectType mResourceType = Resources::ObjectType::Size;
		Resources::IResource* mResourcePtr	= nullptr;
		
		bool mIsConvexShape					= false;
		u64 mResourceHash = 0;
	};
}
