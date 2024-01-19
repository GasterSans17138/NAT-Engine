#pragma once

#include "JoltPhysicsEngine.hpp"

#include <mutex>

namespace Wrappers::PhysicsEngine
{
	class JoltPhysicsEngine;

	struct CollisionBeginStruct
	{
		Core::Scene::Components::Colliders::ICollider* c1;
		Core::Scene::Components::Colliders::ICollider* c2;
		Maths::Vec3 normal;
	};

	struct CollisionStayStruct
	{
		Core::Scene::Components::Colliders::ICollider* c1;
		Core::Scene::Components::Colliders::ICollider* c2;
		Maths::Vec3 normal;
	};

	struct CollisionEndStruct
	{
		Core::Scene::Components::Colliders::ICollider* c1;
		Core::Scene::Components::Colliders::ICollider* c2;
	};

	class NAT_API CollisionEvent : public JPH::ContactListener
	{
	public:
		CollisionEvent(JoltPhysicsEngine* pEngine) : mEngine(pEngine) {};
		~CollisionEvent() = default;

		///First time body are contacting
		JPH::ValidateResult	OnContactValidate(const JPH::Body& inBody1, const JPH::Body& inBody2, const JPH::CollideShapeResult& inCollisionResult) override;

		///Called when two body start
		void OnContactAdded(const JPH::Body& inBody1, const JPH::Body& inBody2, const JPH::ContactManifold& inManifold, JPH::ContactSettings& ioSettings) override;
		void OnContactPersisted(const JPH::Body& inBody1, const JPH::Body& inBody2, const JPH::ContactManifold& inManifold, JPH::ContactSettings& ioSettings) override;

		///When the body pair exit contact 
		void OnContactRemoved(const JPH::SubShapeIDPair& inSubShapePair) override;

		void UpdateEvents();
		void ClearEvents();

	private:
		JoltPhysicsEngine* mEngine = nullptr;
		std::mutex beginListLock;
		std::mutex stayListLock;
		std::mutex endListLock;
		std::vector<CollisionBeginStruct> beginList;
		std::vector<CollisionStayStruct> stayList;
		std::vector<CollisionEndStruct> endList;
	};
}
