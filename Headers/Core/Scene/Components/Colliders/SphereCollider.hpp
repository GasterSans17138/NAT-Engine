#pragma once
#include "Core/Scene/Components/Colliders/ICollider.hpp"


namespace Core::Scene::Components::Colliders
{
	class NAT_API SphereCollider : public ICollider
	{
	public:
		SphereCollider() = default;
		~SphereCollider() = default;

		void Serialize(Core::Serialization::Serializer& sr) const;
		void Deserialize(Core::Serialization::Deserializer& dr);
		void Init() override;

		IComponent* CreateCopy() override;
		void RenderGui() override;
		ComponentType GetType() override;
		const char* GetName() override;
		void OnScaleUpdate(const Maths::Vec3& pSize) override;

	public:
		float radius = 1.f;
	};
}
