#include "Core/App.hpp"

#include "Core/Scene/Components/Colliders/CubeCollider.hpp"

#include "Wrappers/Interfacing.hpp"
#include "Wrappers/PhysicsEngine/JoltPhysicsEngine.hpp"

using namespace Core::Scene::Components;

namespace Core::Scene::Components::Colliders
{
	void CubeCollider::Serialize(Core::Serialization::Serializer& sr) const
	{
		ICollider::Serialize(sr);

		sr.Write(size);
	}

	void CubeCollider::Init()
	{
		if (this->mIsInitialised)
			return;
		ICollider::Init();

		physicEngine->AddCubeCollider(this);

		physicEngine->SetColliderRotation	(this, mColliderRotationOffset);
		physicEngine->SetColliderPosition	(this, mColliderOffset);
		physicEngine->SetColliderScale		(this, size);
		physicEngine->SetMotionType			(this, colliderType);
		physicEngine->SetLayerType			(this, layerType);
		physicEngine->SetColliderTrigger	(this, isTrigger);
	}

	void CubeCollider::Deserialize(Core::Serialization::Deserializer& dr)
	{
		ICollider::Deserialize(dr);

		dr.Read(size);
	}


	IComponent* CubeCollider::CreateCopy()
	{
		auto result = new CubeCollider(*this);
		result->mIsInitialised = false;
		return result;
	}

	void CubeCollider::RenderGui()
	{
		ICollider::RenderGui();
		if (interfaceGui->DragFloat3("Size", &size.x, 0.1f, 0.01f, INFINITY))
		{
			gameObject->ForceUpdate();
		}
	}

	ComponentType CubeCollider::GetType()
	{
		return ComponentType::CubeCollider;
	}

	const char* CubeCollider::GetName()
	{
		return "Cube Collider";
	}

	void CubeCollider::OnStateChanged(const bool& pNewState)
	{

	}

	void CubeCollider::OnScaleUpdate(const Maths::Vec3& pSize)
	{
		physicEngine->SetColliderScale(this, pSize * size);
	}
}