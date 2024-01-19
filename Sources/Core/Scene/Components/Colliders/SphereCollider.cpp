#include "Core/Scene/Components/Colliders/SphereCollider.hpp"
#include "Core/App.hpp"
#include "Wrappers/PhysicsEngine/JoltPhysicsEngine.hpp"

namespace Core::Scene::Components::Colliders
{
	void SphereCollider::Serialize(Core::Serialization::Serializer& sr) const
	{
		ICollider::Serialize(sr);

		sr.Write(radius);
	}

	void SphereCollider::Deserialize(Core::Serialization::Deserializer& dr)
	{
		ICollider::Deserialize(dr);

		dr.Read(radius);
	}
	void SphereCollider::Init()
	{
		if (mIsInitialised)
			return;
		ICollider::Init();

		physicEngine->AddSphereCollider(this);

		physicEngine->SetColliderRotation	(this, mColliderRotationOffset);
		physicEngine->SetColliderPosition	(this, mColliderOffset);
		physicEngine->SetColliderScale		(this, radius);
		physicEngine->SetMotionType			(this, colliderType);
		physicEngine->SetLayerType			(this, layerType);
		physicEngine->SetColliderTrigger(this, isTrigger);

		mIsInitialised = true;
	}

	IComponent* SphereCollider::CreateCopy()
	{
		auto result = new SphereCollider(*this);
		result->mIsInitialised = false;
		return result;
	}

	void SphereCollider::RenderGui()
	{
		ICollider::RenderGui();
		if (interfaceGui->DragFloat("Radius", &radius, 0.1f))
		{
			gameObject->ForceUpdate();
		}
	}

	ComponentType SphereCollider::GetType()
	{
		return ComponentType::SphereCollider;
	}

	const char* SphereCollider::GetName()
	{
		return "Sphere Collider";
	}

	void SphereCollider::OnScaleUpdate(const Maths::Vec3& pSize)
	{
		physicEngine->SetColliderScale(this, Maths::Vec3(Maths::Util::MinF(pSize.x * radius, Maths::Util::MinF(pSize.y * radius, pSize.z * radius))));
	}
}