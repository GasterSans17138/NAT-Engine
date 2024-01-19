#pragma once

#include "Maths/Maths.hpp"
#include "Core/Scene/Components/Colliders/ICollider.hpp"

namespace Wrappers::PhysicsEngine
{
	class JoltPhysicsEngine;
}

namespace Core::Scene::Components::Colliders
{
	class NAT_API CubeCollider : public ICollider
	{
	public:
		CubeCollider() = default;
		~CubeCollider() = default;

		void Serialize(Core::Serialization::Serializer& sr) const;
		void Deserialize(Core::Serialization::Deserializer& dr);
		void Init() override;

		IComponent* CreateCopy() override;
		void RenderGui() override;
		ComponentType GetType() override;
		const char* GetName() override;

		void OnStateChanged(const bool& pNewState) override;

		void OnScaleUpdate(const Maths::Vec3& pSize) override;

	public:
		Maths::Vec3 size = Maths::Vec3(1);
	};
}
