#include "Wrappers/PhysicsEngine/JoltPhysicsEngine.hpp"

#include "Maths/Maths.hpp"

#include "Core/Types.hpp"
#include "Core/App.hpp"
#include "Core/Debugging/Assert.hpp"

#include "Jolt/Jolt.h"
#include "Jolt/RegisterTypes.h"

#include "Jolt/Physics/Body/BodyCreationSettings.h"
#include "Jolt/Physics/Body/BodyManager.h"
#include "Jolt/Physics/Collision/Shape/MeshShape.h"
#include "Jolt/Physics/Collision/Shape/ConvexHullShape.h"

#include "Jolt/Physics/Collision/Shape/BoxShape.h"
#include "Jolt/Physics/Collision/Shape/CapsuleShape.h"
#include "Jolt/Physics/Collision/Shape/SphereShape.h"
#include "Jolt/Physics/Collision/RayCast.h"
#include "Jolt/Physics/Collision/CollisionCollectorImpl.h"
#include "Jolt/Physics/Collision/CastResult.h"


using namespace JPH;


namespace Wrappers::PhysicsEngine
{
	//
	//JoltPhysicsEngine
	//

	/// Should be called before instancing the class.
	void JoltPhysicsEngine::Register()
	{
		RegisterDefaultAllocator();
		Factory::sInstance = new Factory();
		RegisterTypes();
	}

	void JoltPhysicsEngine::Init()
	{
		mPhysicsSystem.Init(MAX_BODIES, BODIES_MUTEX_COUNT, MAX_BODIES_PAIRS, MAX_CONSTRAINTS, mBroadPhaseLayerImpl, &JoltPhysicsEngine::ObjectVsBroadPhaseImpl, &JoltPhysicsEngine::ObjectsCanColide);
		mPhysicsSystem.SetContactListener((ContactListener*)&mCollisionListener);

		mCollidersDrawSettings.mDrawShapeWireframe = true;
	}

	void JoltPhysicsEngine::Init(Renderer::VulkanRenderer* pRenderer)
	{
		mColliderRenderer.Init(pRenderer);
		Init();
	}

	void JoltPhysicsEngine::Update()
	{
		static const Wrappers::WindowManager& window = Core::App::GetInstance()->GetWindow();

		f32 deltaTime = window.GetDeltaTime();

		u32 collisionSteps = static_cast<u32>(Maths::Util::MaxF(std::ceilf(deltaTime*60.0f), 1.0f));

		for (u32 i = 0; i < collisionSteps; i++)
		{
			mPhysicsSystem.Update(deltaTime/collisionSteps, 1, 1, &updateTempAllocation, &mJobSytem);
		}
		mCollisionListener.UpdateEvents();
	}

	void JoltPhysicsEngine::Release()
	{
		mColliderRenderer.Release();
	
		delete Factory::sInstance;
		Factory::sInstance = nullptr;
	}

	void JoltPhysicsEngine::SetGravity(const Maths::Vec3& pGravity)
	{
		mPhysicsSystem.SetGravity(pGravity);
	}

	Maths::Vec3 Wrappers::PhysicsEngine::JoltPhysicsEngine::GetGravity() const
	{
		return mPhysicsSystem.GetGravity();
	}

	void JoltPhysicsEngine::AddForce(const Core::Scene::Components::Colliders::ICollider& pCollider, const Maths::Vec3& pForce)
	{
		mBodyInterface.AddForce(pCollider.bodyID, pForce * coefficientForce);
	}

	void JoltPhysicsEngine::EditorRenderColliders()
	{
		mPhysicsSystem.DrawBodies(mCollidersDrawSettings, &mColliderRenderer);
		mColliderRenderer.Render();
	}

	void JoltPhysicsEngine::EditorClearDrawList()
	{
		mColliderRenderer.ClearDrawList();
	}
	
	void JoltPhysicsEngine::AddCubeCollider(Core::Scene::Components::Colliders::CubeCollider* pComponent)
	{
		Core::Scene::Components::LayerType layer = Core::Scene::Components::LayerType::ALL;

		BodyCreationSettings settings(new BoxShape(Maths::Vec3(1,1,1)), pComponent->gameObject->transform.GetPosition(), pComponent->gameObject->transform.GetRotation(), (EMotionType)pComponent->colliderType, layer);
		settings.mAllowDynamicOrKinematic = true;

		BodyID id = mBodyInterface.CreateAndAddBody(settings, pComponent->gameObject->IsActive() ? EActivation::Activate : EActivation::DontActivate);

		if (id.IsInvalid())
		{
			LOG(DEBUG_LEVEL::LERROR, "JoltPhysicsEngine::AddCubeCollider() : Invalid physic body ID : %u", id.GetIndex());
			Assert(false);
		}

	    pComponent->defaultShape = mBodyInterface.GetShape(id);
		
		SetColliderScale(pComponent, pComponent->size);

		RegisterBody(id, pComponent);

		return;
	}

	void JoltPhysicsEngine::AddSphereCollider(Core::Scene::Components::Colliders::SphereCollider* pComponent)
	{
		Core::Scene::Components::LayerType layer = Core::Scene::Components::LayerType::ALL;

		BodyCreationSettings settings(new SphereShape(1), pComponent->gameObject->transform.GetPosition(), pComponent->gameObject->transform.GetRotation(), (EMotionType)pComponent->colliderType, layer);
		settings.mAllowDynamicOrKinematic = true;

		BodyID id = mBodyInterface.CreateAndAddBody(settings, pComponent->gameObject->IsActive() ? EActivation::Activate : EActivation::DontActivate);

		pComponent->defaultShape = mBodyInterface.GetShape(id);

		SetColliderScale(pComponent, pComponent->radius);

		RegisterBody(id, pComponent);

		return;
	}

	void JoltPhysicsEngine::AddMeshCollider(const Resources::Mesh& pMesh, Core::Scene::Components::Colliders::MeshCollider* pComponent, const bool& pIsConvexShape)
	{
		size_t verticeCount = pMesh.vertices.size();
		size_t indiceCount  = pMesh.indices.size() ;

		Array<Vec3> vertices;
		Array<IndexedTriangle> indexedTriangles;

		if(pIsConvexShape)
			indexedTriangles.reserve((indiceCount / 3) + 1);
		vertices.reserve(verticeCount);

		for (const Renderer::RendererVertex& vertex : pMesh.vertices)
			vertices.push_back(vertex.pos);


		//Grab indices
		for (size_t i = 0; i < pMesh.indices.size(); i+=3)
		{
			indexedTriangles.push_back({ pMesh.indices[i],pMesh.indices[i+1],pMesh.indices[i+2] });
		}

		AddMeshCollider(vertices, indexedTriangles, pComponent, pIsConvexShape);
	}
	
	void JoltPhysicsEngine::AddMeshCollider(const Resources::Model& pModel, Core::Scene::Components::Colliders::MeshCollider* pComponent, const bool& pIsConvexShape)
	{
		size_t verticeCount = 0;
		size_t indicesCount = 0;

		for (const Resources::Mesh* mesh : pModel.meshes)
		{
			verticeCount += mesh->vertices.size();
			indicesCount += mesh->indices.size() ;
		}

		Array<Vec3> vertices;
		
		Array<IndexedTriangle> indexedTriangles;
		if (pIsConvexShape)
			indexedTriangles.reserve((indicesCount / 3) + 1);

		vertices.reserve(verticeCount);

		for (const Resources::Mesh* mesh : pModel.meshes)
		{
			for (const Renderer::RendererVertex& vertex : mesh->vertices)
				vertices.push_back(vertex.pos);

			if (pIsConvexShape)
				break;

			//Grab indices
			for (size_t i = 0; i < mesh->indices.size()-1; i+=3)
			{
				indexedTriangles.push_back({ mesh->indices[i],mesh->indices[i + 1],mesh->indices[i + 2] });
			}
		}

		AddMeshCollider(vertices, indexedTriangles, pComponent, pIsConvexShape);
	}

	void JoltPhysicsEngine::AddMeshCollider(const Array<Vec3>& pVertices, const Array<IndexedTriangle>& pIndexedTriangleList, Core::Scene::Components::Colliders::MeshCollider* pComponent, const bool& pIsConvexShape)
	{
		if (pVertices.size() <= 0) return;
		Core::Scene::Components::LayerType layer = Core::Scene::Components::LayerType::ALL;
		BodyID id;

		if (pIsConvexShape)
		{		
			BodyCreationSettings bodySettings(new ConvexHullShapeSettings(pVertices, cDefaultConvexRadius), pComponent->gameObject->transform.GetPosition(), pComponent->gameObject->transform.GetRotation(), (EMotionType)pComponent->colliderType, layer);
			bodySettings.mAllowDynamicOrKinematic = true;

			id = mBodyInterface.CreateAndAddBody(bodySettings, pComponent->gameObject->IsActive() ? EActivation::Activate : EActivation::DontActivate);
		}
		else
		{
			VertexList vertexList;
			
			vertexList.reserve(pVertices.size());

			//ANOTHER CONVERSION ?
			for (Vec3 vertex : pVertices)
			{
				vertexList.push_back({ vertex.GetX(), vertex.GetY(), vertex.GetZ() });
			}

			BodyCreationSettings bodySettings(new MeshShapeSettings(vertexList, pIndexedTriangleList), pComponent->gameObject->transform.GetPosition(), pComponent->gameObject->transform.GetRotation(), (EMotionType)pComponent->colliderType, layer);
			bodySettings.mAllowDynamicOrKinematic = true;

			bodySettings.mMassPropertiesOverride.mMass = 1;
			bodySettings.mOverrideMassProperties = EOverrideMassProperties::MassAndInertiaProvided;

			id = mBodyInterface.CreateAndAddBody(bodySettings, pComponent->gameObject->IsActive() ? EActivation::Activate : EActivation::DontActivate); 
		}
		
		pComponent->defaultShape = mBodyInterface.GetShape(id);

		RegisterBody(id, pComponent);
	}

	void JoltPhysicsEngine::AddCapsuleCollider(Core::Scene::Components::Colliders::CapsuleCollider* pComponent)
	{
		Core::Scene::Components::LayerType layer = Core::Scene::Components::LayerType::ALL;

		BodyCreationSettings settings(new CapsuleShape(pComponent->height, pComponent->radius), pComponent->gameObject->transform.GetPosition(), pComponent->gameObject->transform.GetRotation(), (EMotionType)pComponent->colliderType, layer);
		settings.mAllowDynamicOrKinematic = true; 

		BodyID id = mBodyInterface.CreateAndAddBody(settings, pComponent->gameObject->IsActive() ? EActivation::Activate : EActivation::DontActivate);

		pComponent->defaultShape = mBodyInterface.GetShape(id);

		SetColliderScale(pComponent, Maths::Vec3(0, pComponent->height, pComponent->radius));

		RegisterBody(id, pComponent);

		return;
	}

	///Un-used
	void JoltPhysicsEngine::AddRigibody(Core::Scene::Components::RigibodyComponent* pComponent)
	{
		return ;
	}

	void JoltPhysicsEngine::SetBodyActiveState(const Core::Scene::Components::IPhysicalComponent* pComponent, const bool& pNewState)
	{
		if (pNewState)
			mBodyInterface.ActivateBody(pComponent->bodyID);
		else
			mBodyInterface.DeactivateBody(pComponent->bodyID);
	}

	Core::Scene::Components::Colliders::ICollider* JoltPhysicsEngine::GetComponentFromID(const BodyID& id)
	{
		auto result = mComponentMap.find(id);
		if (result == mComponentMap.end())
		{
			LOG(DEBUG_LEVEL::LINFO, "Unknown collider %d !", id.GetIndex());
			return nullptr;
		}
		return result->second;
	}

	Maths::Vec3 JoltPhysicsEngine::GetPosition(const Core::Scene::Components::Colliders::ICollider* pCollider)
	{
		return mBodyInterface.GetPosition(pCollider->bodyID);
	}

	Maths::Quat JoltPhysicsEngine::GetColliderRotation(const Core::Scene::Components::Colliders::ICollider* pCollider)
	{
		return mBodyInterface.GetRotation(pCollider->bodyID);
	}

	void JoltPhysicsEngine::ClearEvents()
	{
		mCollisionListener.ClearEvents();
	}

	void JoltPhysicsEngine::MoveColliderTo(const Core::Scene::Components::Colliders::ICollider* pCollider, const Maths::Vec3& targetPos, const Maths::Quat& targetRotation, f32 deltaTime)
	{
		if (pCollider->colliderType == Core::Scene::Components::ColliderType::STATIC) return;
		mBodyInterface.MoveKinematic(pCollider->bodyID, targetPos, targetRotation, deltaTime);
	}

	Maths::Vec3 JoltPhysicsEngine::GetCenterOfMassPosition(const BodyID& inBodyID) const
	{
		return mBodyInterface.GetCenterOfMassPosition(inBodyID);
	}

	Maths::Mat4 JoltPhysicsEngine::GetCenterOfMassTransform(const BodyID& inBodyID) const
	{
		return mBodyInterface.GetCenterOfMassTransform(inBodyID);
	}

	void JoltPhysicsEngine::SetMotionType(const Core::Scene::Components::IPhysicalComponent* pComponent, const Core::Scene::Components::ColliderType& pMotionType)
	{
		mBodyInterface.SetMotionType(pComponent->bodyID, (JPH::EMotionType)pMotionType, pComponent->active);
	}

	void JoltPhysicsEngine::SetLayerType(const Core::Scene::Components::IPhysicalComponent* pComponent, const Core::Scene::Components::LayerType& pLayerType)
	{
		mBodyInterface.SetObjectLayer(pComponent->bodyID, (JPH::ObjectLayer)pLayerType);
	}

	void Wrappers::PhysicsEngine::JoltPhysicsEngine::SetFriction(const Core::Scene::Components::IPhysicalComponent* pComponent, f32 value)
	{
		mBodyInterface.SetFriction(pComponent->bodyID, value);
	}

	void Wrappers::PhysicsEngine::JoltPhysicsEngine::SetBounciness(const Core::Scene::Components::IPhysicalComponent* pComponent, f32 value)
	{
		mBodyInterface.SetRestitution(pComponent->bodyID, value);
	}

	Maths::Vec3 JoltPhysicsEngine::GetBodyVelocity(const Core::Scene::Components::IPhysicalComponent* pComponent)
	{
		return mBodyInterface.GetLinearVelocity(pComponent->bodyID);
	}

	Maths::Vec3 JoltPhysicsEngine::GetBodyAngularVelocity(const Core::Scene::Components::IPhysicalComponent* pComponent)
	{
		return mBodyInterface.GetAngularVelocity(pComponent->bodyID);
	}

	void JoltPhysicsEngine::SetBodyVelocity(const Core::Scene::Components::IPhysicalComponent* pComponent, const Maths::Vec3& pNewVel)
	{
		mBodyInterface.SetLinearVelocity(pComponent->bodyID, pNewVel);
	}

	void JoltPhysicsEngine::SetBodyAngularVelocity(const Core::Scene::Components::IPhysicalComponent* pComponent, const Maths::Vec3& pNewVel)
	{
		mBodyInterface.SetAngularVelocity(pComponent->bodyID, pNewVel);
	}

	void JoltPhysicsEngine::SetColliderPosition(const Core::Scene::Components::Colliders::ICollider* pCollider, const Maths::Vec3& pPosition)
	{
		mBodyInterface.SetPosition(pCollider->bodyID, pPosition, pCollider->active);
	}

	void JoltPhysicsEngine::SetColliderRotation(const Core::Scene::Components::Colliders::ICollider* pCollider, const Maths::Quat& inRotation)
	{
		mBodyInterface.SetRotation(pCollider->bodyID, inRotation, pCollider->active);
	}
	
	void JoltPhysicsEngine::SetColliderScale(const Core::Scene::Components::Colliders::ICollider* pCollider, const Maths::Vec3& pSize)
	{
		if (pSize.x <= 0 || pSize.y <= 0 || pSize.z <= 0) return; //No negative scale
		mBodyInterface.SetShape(pCollider->bodyID, pCollider->defaultShape->ScaleShape(Maths::Util::MaxV(pSize, 0.00001f)).Get(), true, pCollider->active);
	}

	void JoltPhysicsEngine::SetCapsuleHeight(Core::Scene::Components::Colliders::CapsuleCollider* pCapsule, const float& pHeight)
	{
		if (pHeight <= 0) return;

		pCapsule->defaultShape = CapsuleShapeSettings(pHeight/2, pCapsule->radius).Create().Get();
		this->SetColliderScale(pCapsule, Maths::Vec3(1.0f));
	}

	void JoltPhysicsEngine::SetCapsuleRadius(Core::Scene::Components::Colliders::CapsuleCollider* pCapsule, const float& pRadius)
	{
		if (pRadius <= 0) return;

		pCapsule->defaultShape = CapsuleShapeSettings(pCapsule->height/2, pRadius).Create().Get();
		this->SetColliderScale(pCapsule, Maths::Vec3(1.0f));
	}

	void JoltPhysicsEngine::SetColliderScale(const Core::Scene::Components::Colliders::ICollider* pCollider, const float& pSize)
	{
		mBodyInterface.SetShape(pCollider->bodyID, pCollider->defaultShape->ScaleShape(Maths::Vec3(pSize)).Get(), true, pCollider->active);
	}

	bool JoltPhysicsEngine::SetColliderTrigger(const Core::Scene::Components::Colliders::ICollider* pCollider, bool isTrigger)
	{
		JPH::BodyLockWrite lock(mPhysicsSystem.GetBodyLockInterface(), pCollider->bodyID);
		if (lock.Succeeded()) // body_id may no longer be valid
		{
			JPH::Body& body = lock.GetBody();
			body.SetIsSensor(isTrigger);
			return true;
		}
		return false;
	}

	bool JoltPhysicsEngine::IsColliderTrigger(const Core::Scene::Components::Colliders::ICollider* pCollider)
	{
		JPH::BodyLockRead lock(mPhysicsSystem.GetBodyLockInterface(), pCollider->bodyID);
		if (lock.Succeeded()) // body_id may no longer be valid
		{
			const JPH::Body& body = lock.GetBody();
			return body.IsSensor();
		}
		return false;
	}

	bool JoltPhysicsEngine::ColliderIsActive(const Core::Scene::Components::Colliders::ICollider* pCollider) const
	{
		return mBodyInterface.IsActive(pCollider->bodyID);
	}

	void JoltPhysicsEngine::RegisterBody(const BodyID pBody, Core::Scene::Components::Colliders::ICollider* pComponent)
	{
		if (pBody.IsInvalid())
		{
			LOG(DEBUG_LEVEL::LERROR, "JoltPhysicsEngine::AddCubeCollider : Error while creating body. Please double check the code.");
			Assert(false);
		}

		if (mComponentMap.find(pBody) != mComponentMap.end())
		{
			LOG(DEBUG_LEVEL::LWARNING, "JoltPhysicsEngine::RegisterBody : Cant register body, it already exist somewhere (collision events might be broken)");
			return;
		}
		
		pComponent->bodyID = pBody;
		mComponentMap.emplace(pBody, pComponent);
	}

	void JoltPhysicsEngine::UnRegisterBody(const BodyID& pBody)
	{
		mComponentMap.erase(pBody);
	}

	bool JoltPhysicsEngine::ObjectVsBroadPhaseImpl(ObjectLayer inLayer1, BroadPhaseLayer inLayer2)
	{
		return true;
	}

	bool JoltPhysicsEngine::ObjectsCanColide(ObjectLayer inObject1, ObjectLayer inObject2)
	{
		return inObject1 & inObject2;
	}

	bool JoltPhysicsEngine::RayCast(Maths::Vec3 from, Maths::Vec3 direction, RayCastResult& result, u16 layer)
	{
		return RayCast(from, direction, result, static_cast<Core::Scene::Components::LayerType>(layer));
	}

	VulkanColliderRenderer& JoltPhysicsEngine::GetDebugRenderer()
	{
		return mColliderRenderer;
	}

	void JoltPhysicsEngine::RemovedBodyFromSimulation(const Core::Scene::Components::IPhysicalComponent* pComponent)
	{
		mBodyInterface.RemoveBody(pComponent->bodyID);
	mComponentMap.erase(pComponent->bodyID);
	}

	bool JoltPhysicsEngine::RayCast(Maths::Vec3 from, Maths::Vec3 direction, RayCastResult& result, Core::Scene::Components::LayerType layer)
	{
		JPH::RayCast ray{from, direction};
		JPH::RayCastResult castResult;
		mColliderRenderer.DrawLine(from, from + direction, ColorArg(0, 255, 0));
		if (!mPhysicsSystem.GetNarrowPhaseQuery().CastRay(ray, castResult, {}, BitmaskObjectLayerFilter(layer))) return false;
		result.hitCollider = GetComponentFromID(castResult.mBodyID);
		result.position = ray.GetPointOnRay(castResult.mFraction);
		result.normal = result.hitCollider->defaultShape->GetSurfaceNormal(castResult.mSubShapeID2, result.position - GetCenterOfMassPosition(castResult.mBodyID));
		mColliderRenderer.DrawArrow(result.position, result.position + result.normal, ColorArg(0, 0, 255), 1.0f);
		return true;
		/*
		{
			JPH::BodyLockRead lock(mPhysicsSystem.GetBodyLockInterface(), result.mBodyID);
			if (lock.Succeeded()) // body_id may no longer be valid
			{
				const JPH::Body& body = lock.GetBody();
				body.GetWorldSpaceSurfaceNormal(result.mSubShapeID2, ray.GetPointOnRay(result.mFraction));
				return true;
			}
		}
		return false;
		*/
	}
}
