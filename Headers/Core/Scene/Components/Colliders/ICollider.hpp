#pragma once

#include "Maths/Maths.hpp"
#include "Core/Scene/Transform.hpp"

#include "Core/Scene/Components/IPhysicalComponent.hpp"

namespace Core::Scene::Components::Colliders
{
	class NAT_API ICollider : public IPhysicalComponent
	{
	public:
		ICollider() = default;
		~ICollider() = default;

		virtual void Serialize(Core::Serialization::Serializer& sr) const override;
		virtual void Deserialize(Core::Serialization::Deserializer& dr) override;

		virtual void RenderGui() override;

		/// Please ALWAYS call the parent's Update/Init/Constructor or any other data-manipulation function when overriding
		virtual void Init() override;
		virtual void DataUpdate() override;

	public :

		Maths::Vec3	GetCenterOfMassPosition() const;
		Maths::Mat4	GetCenterOfMassTransform() const;

		Maths::Vec3	GetPositionOffset() const;
		Maths::Quat	GetRotationOffset() const;

		void SetMotionType(const ColliderType& pColliderType);
		void SetLayerType(const LayerType& pLayerType);
		void SetFriction(f32 value);
		void SetBounciness(f32 value);

		f32 GetBounciness(f32) const;
		f32 GetFriction(f32) const;

		virtual void OnStateChanged(const bool& pNewState) override;

		virtual void OnPositionUpdate(const Maths::Vec3& newPos) override;
		virtual void OnRotationUpdate(const Maths::Quat& newPos) override;
		virtual void OnScaleUpdate(const Maths::Vec3& newPos) override;

		bool ColliderIsActive() const;
		
		enum CollisionType
		{
			Discrete,
			Linear
		};
		
		void			SetUpdateType(const CollisionType& pCollisionType);
		CollisionType	GetCollisionType() const;

	protected :
		Maths::Vec3 mColliderOffset;
		Maths::Quat mColliderRotationOffset;

		float mFriction = 1.f;
		float mBounciness = 0.5f;
	};
}
