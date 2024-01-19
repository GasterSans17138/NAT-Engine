#include "Core/App.hpp"

#include "Core/Scene/Components/Colliders/CapsuleCollider.hpp"

namespace Core::Scene::Components::Colliders
{
	void CapsuleCollider::Serialize(Core::Serialization::Serializer& sr) const
	{
		ICollider::Serialize(sr);
		
		sr.Write(height);
		sr.Write(radius);
	}

	void CapsuleCollider::Deserialize(Core::Serialization::Deserializer& dr)
	{
		ICollider::Deserialize(dr);

		dr.Read(height);
		dr.Read(radius);
	}

	void CapsuleCollider::Init()
	{
		if (mIsInitialised)
			return;
		ICollider::Init();

		physicEngine->AddCapsuleCollider(this);

		physicEngine->SetColliderRotation	(this, mColliderRotationOffset);
		physicEngine->SetColliderPosition	(this, mColliderOffset);
		physicEngine->SetCapsuleHeight		(this, height);
		physicEngine->SetCapsuleRadius		(this, radius);
		physicEngine->SetMotionType			(this, colliderType);
		physicEngine->SetLayerType			(this, layerType);
		physicEngine->SetColliderTrigger	(this, isTrigger);

		mIsInitialised = true;
	}

	IComponent* CapsuleCollider::CreateCopy()
	{
		auto result = new CapsuleCollider(*this);
		result->mIsInitialised = false;
		return result;
	}

	void CapsuleCollider::RenderGui()
	{
		ICollider::RenderGui();

		bool edited = false;

		if (interfaceGui->DragFloat( "Capsule Height" , &height, 0.1f, 0.0001f, INFINITY))	physicEngine->SetCapsuleHeight(this, height);
		if (interfaceGui->DragFloat("Capsule Diameter", &radius, 0.1f, 0.0001f, INFINITY))	physicEngine->SetCapsuleRadius(this, radius);
	}

	ComponentType CapsuleCollider::GetType()
	{
		return ComponentType::CapsuleCollider;
	}

	const char* CapsuleCollider::GetName()
	{
		return "Capsule Collider";
	}

	void CapsuleCollider::OnScaleUpdate(const Maths::Vec3& pSize)
	{

	}
}