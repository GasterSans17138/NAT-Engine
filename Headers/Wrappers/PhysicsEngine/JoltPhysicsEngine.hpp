#pragma once

#include <thread>

#include "Jolt/Jolt.h"

#include "Jolt/Physics/PhysicsSystem.h"
#include "Jolt/Core/JobSystemThreadPool.h"
#include "Jolt/Core/TempAllocator.h"
#include "Jolt/Core/Factory.h"
#include "Jolt/Physics/Collision/ObjectLayer.h"

#include "Resources/Model.hpp"

#include "Core/Debugging/Assert.hpp"

#include "Core/Scene/GameObject.hpp"

#include "Core/Scene/Components/Colliders/CubeCollider.hpp"
#include "Core/Scene/Components/Colliders/SphereCollider.hpp"
#include "Core/Scene/Components/Colliders/CapsuleCollider.hpp"
#include "Core/Scene/Components/Colliders/MeshCollider.hpp"
#include "Core/Scene/Components/RigibodyComponent.hpp"

#include "Wrappers/WindowManager.hpp"

#include "Maths/Maths.hpp"

#include "BroadPhaseLayer.hpp"
#include "CollisionEvent.hpp"
#include "VulkanColliderRenderer.hpp"

#include "IPhysicsEngine.hpp"

#define MAX_BODIES			65536
#define MAX_BODIES_PAIRS	65536

#define MAX_CONSTRAINTS		10240

#define BODIES_MUTEX_COUNT	0						//0 for autodetect

#define UPDATE_MEMORY_ALLOC_SIZE 16 * 1024 * 1024	//64 mb, probably too much

#define MAX_CONCURENT_JOBS JPH::cMaxPhysicsJobs
#define MAX_BARRIERS JPH::cMaxPhysicsJobs

namespace Wrappers::PhysicsEngine
{
	class NAT_API BitmaskObjectLayerFilter : public JPH::ObjectLayerFilter
	{
	public:
		/// Constructor
		explicit				BitmaskObjectLayerFilter(u16 inLayer) :
			mLayer(inLayer)
		{
		}

		// See ObjectLayerFilter::ShouldCollide
		virtual bool			ShouldCollide(JPH::ObjectLayer inLayer) const override
		{
			return mLayer & inLayer; // All of that to just change this operator...
		}

	private:
		u16	mLayer;
	};

	class NAT_API JoltPhysicsEngine final : public IPhysicsEngine
	{
	public:
		//
		//IPhysicsEngine
		//

		JoltPhysicsEngine()  = default;
		~JoltPhysicsEngine() = default;

		void Init(Renderer::VulkanRenderer* pRenderer);
		void Update() override;
		void Release() override;

		void SetGravity(const Maths::Vec3& pGravity) override;
		Maths::Vec3 GetGravity() const override;
		void AddForce(const Core::Scene::Components::Colliders::ICollider& pCollider, const Maths::Vec3& pForce) override;
		
		void EditorRenderColliders() override;
		void EditorClearDrawList() override;
		
		void SetColliderPosition(const Core::Scene::Components::Colliders::ICollider* pCollider, const Maths::Vec3& pPosition)		override;
		void SetColliderRotation(const Core::Scene::Components::Colliders::ICollider* pCollider, const Maths::Quat& inRotation)		override;
		void SetColliderScale	(const Core::Scene::Components::Colliders::ICollider* pCollider, const Maths::Vec3& pScale)			override;
		void SetColliderScale(const Core::Scene::Components::Colliders::ICollider* pCollider, const float& pSize);

		bool SetColliderTrigger(const Core::Scene::Components::Colliders::ICollider* pCollider, bool isTrigger);
		bool IsColliderTrigger(const Core::Scene::Components::Colliders::ICollider* pCollider);

		Maths::Vec3 GetPosition			(const Core::Scene::Components::Colliders::ICollider* pCollider) override;
		Maths::Quat GetColliderRotation	(const Core::Scene::Components::Colliders::ICollider* pCollider) override;

		void ClearEvents() override;

		void MoveColliderTo(const Core::Scene::Components::Colliders::ICollider* pCollider, const Maths::Vec3& targetPos, const Maths::Quat& targetRotation, f32 deltaTime);

		bool ColliderIsActive(const Core::Scene::Components::Colliders::ICollider* pCollider) const override;

		void SetMotionType	(const Core::Scene::Components::IPhysicalComponent* pComponent, const Core::Scene::Components::ColliderType& pMotionType) override;
		void SetLayerType	(const Core::Scene::Components::IPhysicalComponent* pComponent, const Core::Scene::Components::LayerType& pLayerType) override;
		void SetFriction	(const Core::Scene::Components::IPhysicalComponent* pComponent, f32 value) override;
		void SetBounciness	(const Core::Scene::Components::IPhysicalComponent* pComponent, f32 value) override;

		Maths::Vec3 GetBodyVelocity(const Core::Scene::Components::IPhysicalComponent* pComponent)			override;
		Maths::Vec3 GetBodyAngularVelocity(const Core::Scene::Components::IPhysicalComponent* pComponent)	override;

		void SetBodyVelocity(const Core::Scene::Components::IPhysicalComponent* pComponent, const Maths::Vec3& pNewVel) override;
		void SetBodyAngularVelocity(const Core::Scene::Components::IPhysicalComponent* pComponent, const Maths::Vec3& pNewVel)	override;

		void RemovedBodyFromSimulation(const Core::Scene::Components::IPhysicalComponent* pComponent) override;

		void AddMeshCollider	(const Resources::Mesh& pMesh,	 Core::Scene::Components::Colliders::MeshCollider* pComponent, const bool& pIsConvexShape = false)	override;
		void AddMeshCollider	(const Resources::Model& pModel, Core::Scene::Components::Colliders::MeshCollider* pComponent, const bool& pIsConvexShape = false)	override;
		void AddCapsuleCollider	(Core::Scene::Components::Colliders::CapsuleCollider* pComponent)																	override;
		void AddSphereCollider	(Core::Scene::Components::Colliders::SphereCollider* pComponent)																	override;
		void AddCubeCollider	(Core::Scene::Components::Colliders::CubeCollider* pComponent)																		override;
		void AddRigibody		(Core::Scene::Components::RigibodyComponent* pComponent)																			override;

		void SetBodyActiveState(const Core::Scene::Components::IPhysicalComponent* pComponent, const bool& pNewState) override;

		//
		//JoltPhysicsEngine
		//

		static void Register();

		void SetCapsuleHeight(Core::Scene::Components::Colliders::CapsuleCollider* pCapsule, const float& pHeight);
		void SetCapsuleRadius(Core::Scene::Components::Colliders::CapsuleCollider* pCapsule, const float& pRadius);

		Maths::Vec3	GetCenterOfMassPosition(const JPH::BodyID& inBodyID) const;
		Maths::Mat4	GetCenterOfMassTransform(const JPH::BodyID& inBodyID) const;

		Core::Scene::Components::Colliders::ICollider* GetComponentFromID(const JPH::BodyID& id);

		bool RayCast(Maths::Vec3 from, Maths::Vec3 direction, RayCastResult& result, Core::Scene::Components::LayerType layer = Core::Scene::Components::LayerType::ALL) override;
		bool RayCast(Maths::Vec3 from, Maths::Vec3 direction, RayCastResult& result, u16 layer);

		VulkanColliderRenderer& GetDebugRenderer();
	private:

		void Init() override;

		void AddMeshCollider(const JPH::Array<JPH::Vec3>& pVertices, const JPH::Array<JPH::IndexedTriangle>& pIndexedTriangleList, Core::Scene::Components::Colliders::MeshCollider* pComponent, const bool& pIsConvexShape);

		void RegisterBody(const JPH::BodyID pBody, Core::Scene::Components::Colliders::ICollider* pComponent);
		void UnRegisterBody(const JPH::BodyID& pBody);

		static bool ObjectVsBroadPhaseImpl(JPH::ObjectLayer inLayer1, JPH::BroadPhaseLayer inLayer2);
		static bool ObjectsCanColide(JPH::ObjectLayer inObject1, JPH::ObjectLayer inObject2);


	private:
		JPH::PhysicsSystem			mPhysicsSystem;
		JPH::Factory				mFactory;
		JPH::JobSystemThreadPool	mJobSytem				= JPH::JobSystemThreadPool(MAX_CONCURENT_JOBS, MAX_BARRIERS, std::thread::hardware_concurrency() - 1);

		JPH::BodyInterface&			mBodyInterface			= mPhysicsSystem.GetBodyInterface();

		JPH::TempAllocatorImpl		updateTempAllocation	= JPH::TempAllocatorImpl(UPDATE_MEMORY_ALLOC_SIZE);

		BroadPhaseLayerImpl			mBroadPhaseLayerImpl;
		CollisionEvent				mCollisionListener		= CollisionEvent(this);


		using ComponentMap = std::unordered_map<JPH::BodyID, Core::Scene::Components::Colliders::ICollider*>;

		///Keep track of component associated with each body in the simulation
		ComponentMap mComponentMap;


		JPH::BodyManager::DrawSettings mCollidersDrawSettings;
		VulkanColliderRenderer mColliderRenderer;
		f32 coefficientForce = 1000000;
	};
}