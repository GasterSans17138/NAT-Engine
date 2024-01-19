#pragma once

#include "Core/Scene/Components/Colliders/ICollider.hpp"

namespace Core::Scene::Components::Colliders
{
	class NAT_API CapsuleCollider : public ICollider
	{
	public:
		CapsuleCollider() = default;
		~CapsuleCollider() = default;

		float height = 2.f;
		float radius = 1.f;

		void Serialize(Core::Serialization::Serializer& sr) const override;
		void Deserialize(Core::Serialization::Deserializer& dr) override;
		void Init() override;

		IComponent* CreateCopy() override;
		void RenderGui() override;
		ComponentType GetType() override;
		const char* GetName() override;

		void OnScaleUpdate(const Maths::Vec3& pSize) override;
	};
}
