#pragma once

#include "Core/Scene/Components/Colliders/CapsuleCollider.hpp"
#include "Core/Scene/Components/Colliders/MeshCollider.hpp"
#include "Core/Scene/Components/Colliders/SphereCollider.hpp"
#include "Core/Scene/Components/Colliders/CubeCollider.hpp"
#include "Core/Scene/Components/RigibodyComponent.hpp"

#include "Core/Scene/Components/IPhysicalComponent.hpp"

#include "Maths/Maths.hpp"

namespace Wrappers
{
	struct RayCastResult
	{
		Core::Scene::Components::Colliders::ICollider* hitCollider = nullptr;
		Maths::Vec3 position;
		Maths::Vec3 normal;
	};

	class NAT_API IPhysicsEngine
	{
	public:
		IPhysicsEngine() = default;
		~IPhysicsEngine() = default;

		virtual void Init()		= 0;
		virtual void Update()	= 0;
		virtual void Release()	= 0;

		virtual void EditorRenderColliders()	= 0;
		virtual void EditorClearDrawList()		= 0;

		virtual void AddMeshCollider(const Resources::Model& pModel, Core::Scene::Components::Colliders::MeshCollider* pComponent, const bool& pIsConvexShape = false)	= 0;
		virtual void AddMeshCollider(const Resources::Mesh& pMesh,   Core::Scene::Components::Colliders::MeshCollider* pComponent, const bool& pIsConvexShape = false)	= 0;
		virtual void AddCapsuleCollider(Core::Scene::Components::Colliders::CapsuleCollider* pComponent)																= 0;
		virtual void AddSphereCollider(Core::Scene::Components::Colliders::SphereCollider* pComponent)																	= 0;
		virtual void AddCubeCollider(Core::Scene::Components::Colliders::CubeCollider* pComponent)																		= 0;
		virtual void AddRigibody(Core::Scene::Components::RigibodyComponent* pComponent)																				= 0;

		virtual void SetGravity (const Maths::Vec3& pGravity) = 0;
		virtual Maths::Vec3 GetGravity() const = 0;
		virtual void ClearEvents() = 0;

		virtual bool ColliderIsActive(const Core::Scene::Components::Colliders::ICollider* pCollider) const				= 0;

		virtual void AddForce(const Core::Scene::Components::Colliders::ICollider& pBodyID, const Maths::Vec3& pForce)	= 0;

		virtual Maths::Vec3 GetPosition(const Core::Scene::Components::Colliders::ICollider* pCollider)			= 0;
		virtual Maths::Quat GetColliderRotation(const Core::Scene::Components::Colliders::ICollider* pCollider)	= 0;

		virtual void SetColliderScale   (const Core::Scene::Components::Colliders::ICollider* pCollider, const Maths::Vec3& pScale)			= 0;
		virtual void SetColliderPosition(const Core::Scene::Components::Colliders::ICollider* pCollider, const Maths::Vec3& pPosition)		= 0;
		virtual void SetColliderRotation(const Core::Scene::Components::Colliders::ICollider* pCollider, const Maths::Quat& inRotation)		= 0;

		virtual void SetMotionType(const Core::Scene::Components::IPhysicalComponent* pComponent, const Core::Scene::Components::ColliderType& pMotionType)	= 0;
		virtual void SetLayerType(const Core::Scene::Components::IPhysicalComponent* pComponent,  const Core::Scene::Components::LayerType& pLayerType)		= 0;
		virtual void SetFriction(const Core::Scene::Components::IPhysicalComponent* pComponent, f32 value) = 0;
		virtual void SetBounciness(const Core::Scene::Components::IPhysicalComponent* pComponent, f32 value) = 0;

		virtual Maths::Vec3 GetBodyVelocity(const Core::Scene::Components::IPhysicalComponent* pComponent)			= 0;
		virtual Maths::Vec3 GetBodyAngularVelocity(const Core::Scene::Components::IPhysicalComponent* pComponent)	= 0;

		virtual bool RayCast(Maths::Vec3 from, Maths::Vec3 direction, RayCastResult& result, Core::Scene::Components::LayerType layer = Core::Scene::Components::LayerType::ALL) = 0;

		virtual void SetBodyVelocity(const Core::Scene::Components::IPhysicalComponent* pComponent, const Maths::Vec3& pNewVel) = 0;
		virtual void SetBodyAngularVelocity(const Core::Scene::Components::IPhysicalComponent* pComponent, const Maths::Vec3& pNewVel) = 0;

		virtual void RemovedBodyFromSimulation(const Core::Scene::Components::IPhysicalComponent* pCollider) = 0;

		virtual void SetBodyActiveState(const Core::Scene::Components::IPhysicalComponent* pComponent, const bool& pNewState) = 0;
	};
}