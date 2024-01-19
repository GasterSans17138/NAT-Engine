#include "Wrappers/PhysicsEngine/JoltPhysicsEngine.hpp"

#include "Core/Scene/Components/Colliders/ICollider.hpp"

#include "Core/App.hpp"

namespace Core::Scene::Components::Colliders
{


	void ICollider::Serialize(Core::Serialization::Serializer& sr) const
	{
		IPhysicalComponent::Serialize(sr);

		sr.Write(mColliderOffset);
		sr.Write(mColliderRotationOffset);
		sr.Write(mFriction);
		sr.Write(mBounciness);
	}

	void ICollider::Deserialize(Core::Serialization::Deserializer& dr)
	{
		IPhysicalComponent::Deserialize(dr);

		dr.Read(mColliderOffset);
		dr.Read(mColliderRotationOffset);
		dr.Read(mFriction);
		dr.Read(mBounciness);
	}

	void ICollider::RenderGui()
	{
		IPhysicalComponent::RenderGui();
		if (interfaceGui->CheckBox("Is Trigger", &isTrigger))
		{
			physicEngine->SetColliderTrigger(this, isTrigger);
		}
		if (interfaceGui->DragFloat3("Position Offset", &mColliderOffset.x, 0.1f))
		{
			gameObject->ForceUpdate();
		}
		if (interfaceGui->DragQuat("Rotation Offset", mColliderRotationOffset, 0.1f))
		{
			gameObject->ForceUpdate();
		}
	}

	void ICollider::Init()
	{
		if (mIsInitialised)
			return;

		IPhysicalComponent::Init();
	}

	void ICollider::DataUpdate()
	{
		if (ColliderIsActive())
		{
			auto mat = gameObject->parent ? gameObject->parent->transform.GetGlobal().CreateInverseMatrix() : Maths::Mat4::Identity();
			gameObject->transform.SetRotation((physicEngine->GetColliderRotation(this) * Maths::Quat(mat) * mColliderRotationOffset.Inverse()).Normalize());
			gameObject->transform.SetPosition(physicEngine->GetPosition(this) + mat.GetPositionFromTranslation() - Maths::Quat(gameObject->transform.GetGlobal()) * mColliderOffset);
		}
	}

	Maths::Vec3 ICollider::GetCenterOfMassPosition() const
	{
		return physicEngine->GetCenterOfMassPosition(bodyID);
	}

	Maths::Mat4 ICollider::GetCenterOfMassTransform() const
	{
		return physicEngine->GetCenterOfMassTransform(bodyID);
	}

	Maths::Vec3 ICollider::GetPositionOffset() const
	{
		return mColliderOffset;
	}

	Maths::Quat ICollider::GetRotationOffset() const
	{
		return mColliderRotationOffset;
	}

	void ICollider::SetMotionType(const ColliderType& pType)
	{
		this->colliderType = pType;
		physicEngine->SetMotionType(this, colliderType);
	}

	void ICollider::SetLayerType(const LayerType& pLayerType)
	{
		this->layerType = pLayerType;
		physicEngine->SetLayerType(this, layerType);
	}

	void ICollider::SetFriction(f32 value)
	{
		mFriction = value;
		physicEngine->SetFriction(this, mFriction);
	}

	void ICollider::SetBounciness(f32 value)
	{
		mBounciness = value;
		physicEngine->SetBounciness(this, mBounciness);
	}

	f32 ICollider::GetBounciness(f32) const
	{
		return mBounciness;
	}

	f32 ICollider::GetFriction(f32) const
	{
		return mFriction;
	}

	void ICollider::OnStateChanged(const bool& pNewState)
	{
		physicEngine->SetBodyActiveState(this, pNewState);
	}

	void ICollider::OnPositionUpdate(const Maths::Vec3& pPosition)
	{
		physicEngine->SetColliderPosition(this, pPosition + Maths::Quat(gameObject->transform.GetGlobal()) * mColliderOffset);
	}

	void ICollider::OnRotationUpdate(const Maths::Quat& pRotation)
	{
		physicEngine->SetColliderRotation(this, pRotation * mColliderRotationOffset);
	}

	void ICollider::OnScaleUpdate(const Maths::Vec3& pSize)
	{
		physicEngine->SetColliderScale(this, pSize);
	}

	bool ICollider::ColliderIsActive() const
	{
		return physicEngine->ColliderIsActive(this);
	}

	void ICollider::SetUpdateType(const CollisionType& pCollisionType)
	{

	}

	ICollider::CollisionType ICollider::GetCollisionType() const
	{
		return ICollider::CollisionType::Linear;
		//Use JPH::EMotionQuality
	}
}