#include "Core/Scene/GameObject.hpp"

#include "Wrappers/PhysicsEngine/JoltPhysicsEngine.hpp"
#include "Wrappers/PhysicsEngine/CollisionEvent.hpp"

namespace Wrappers::PhysicsEngine
{
	JPH::ValidateResult	CollisionEvent::OnContactValidate(const JPH::Body& inBody1, const JPH::Body& inBody2, const JPH::CollideShapeResult& inCollisionResult)
	{
		return JPH::ValidateResult::AcceptAllContactsForThisBodyPair;
	}

	// Called when two colliders begin contact
	void CollisionEvent::OnContactAdded(const JPH::Body& inBody1, const JPH::Body& inBody2, const JPH::ContactManifold& inManifold, JPH::ContactSettings& ioSettings)
	{
		CollisionBeginStruct event;
		event.c1 = mEngine->GetComponentFromID(inBody1.GetID());
		event.c2 = mEngine->GetComponentFromID(inBody2.GetID());
		event.normal = inManifold.mWorldSpaceNormal;
		if (event.c1 && event.c2)
		{
			beginListLock.lock();
			beginList.push_back(std::move(event));
			beginListLock.unlock();
		}
	}

	// Called when colliders continue touching
	void CollisionEvent::OnContactPersisted(const JPH::Body& inBody1, const JPH::Body& inBody2, const JPH::ContactManifold& inManifold, JPH::ContactSettings& ioSettings)
	{
		CollisionStayStruct event;
		event.c1 = mEngine->GetComponentFromID(inBody1.GetID());
		event.c2 = mEngine->GetComponentFromID(inBody2.GetID());
		event.normal = inManifold.mWorldSpaceNormal;
		if (event.c1 && event.c2)
		{
			stayListLock.lock();
			stayList.push_back(std::move(event));
			stayListLock.unlock();
		}
	}

	// Called when colliders stop touching
	void CollisionEvent::OnContactRemoved(const JPH::SubShapeIDPair& inSubShapePair)
	{
		CollisionEndStruct event;
		event.c1 = mEngine->GetComponentFromID(inSubShapePair.GetBody1ID());
		event.c2 = mEngine->GetComponentFromID(inSubShapePair.GetBody2ID());
		if (event.c1 && event.c2)
		{
			endListLock.lock();
			endList.push_back(std::move(event));
			endListLock.unlock();
		}
	}

	void CollisionEvent::ClearEvents()
	{
		beginListLock.lock();
		beginList.clear();
		beginListLock.unlock();
		stayListLock.lock();
		stayList.clear();
		stayListLock.unlock();
		endListLock.lock();
		endList.clear();
		endListLock.unlock();
	}

	void CollisionEvent::UpdateEvents()
	{
		beginListLock.lock();
		for (auto& event : beginList)
		{
			Core::Scene::GameObject* gameObject1 = event.c1->gameObject;
			Core::Scene::GameObject* gameObject2 = event.c2->gameObject;
			gameObject1->OnCollisionBegin(event.c1, gameObject2, event.c2, event.normal);
			gameObject2->OnCollisionBegin(event.c2, gameObject1, event.c1, -event.normal);
		}
		beginList.clear();
		beginListLock.unlock();
		stayListLock.lock();
		for (auto& event : stayList)
		{
			Core::Scene::GameObject* gameObject1 = event.c1->gameObject;
			Core::Scene::GameObject* gameObject2 = event.c2->gameObject;
			gameObject1->OnCollisionStay(event.c1, gameObject2, event.c2, event.normal);
			gameObject2->OnCollisionStay(event.c2, gameObject1, event.c1, -event.normal);
		}
		stayList.clear();
		stayListLock.unlock();
		endListLock.lock();
		for (auto& event : endList)
		{
			Core::Scene::GameObject* gameObject1 = event.c1->gameObject;
			Core::Scene::GameObject* gameObject2 = event.c2->gameObject;
			gameObject1->OnCollisionEnd(event.c1, gameObject2, event.c2);
			gameObject2->OnCollisionEnd(event.c2, gameObject1, event.c1);
		}
		endList.clear();
		endListLock.unlock();
	}
}
