#pragma once

#include "Jolt/Jolt.h"

#include "Jolt/Physics/Body/BodyID.h"
#include "Jolt/Physics/EActivation.h"
#include "Jolt/Renderer/DebugRenderer.h"
#include "Jolt/Physics/Collision/Shape/Shape.h"

#include "Core/Scene/Components/IComponent.hpp"

namespace Wrappers::PhysicsEngine
{
	class JoltPhysicsEngine;
}

namespace Core::Scene::Components
{
	///This will be a dropdown in engine for the user to select
	enum ColliderType : u32
	{
		STATIC = 0,
		KINEMATIC = 1,
		DYNAMIC = 2
	};

	enum LayerType : u16
	{
		GROUND = 1,
		WALL = 2,
		CEILING = 4,
		OBJECT = 8,
		PORTAL = 16,
		TRIGGER = 32,
		BUTTON = 64,
		ALLBUTPLAYER = 128,
		ALL = static_cast<u16>(~0),
	};

    class NAT_API IPhysicalComponent : public IComponent
    {
	public :
		 IPhysicalComponent() = default;
		~IPhysicalComponent() = default;

		virtual void Init() override;
		virtual void Delete() override;
		virtual void RenderGui() override;

		virtual void Serialize(Core::Serialization::Serializer& sr) const override;
		virtual void Deserialize(Core::Serialization::Deserializer& dr) override;

		void SetMotionType(const ColliderType& pType);
		void SetLayerType(const LayerType& pType);

		Maths::Vec3 GetVelocity() const;
		Maths::Vec3 GetAngularVelocity() const;

		void SetVelocity(Maths::Vec3 pNewVel);
		void SetAngularVelocity(Maths::Vec3 pNewVel);

	public:
		LayerType layerType = LayerType::ALL;
		ColliderType colliderType = ColliderType::STATIC;

		JPH::BodyID bodyID = static_cast<JPH::BodyID>(0);
		JPH::EActivation active = static_cast<JPH::EActivation>(0);
		JPH::RefConst<JPH::Shape> defaultShape;
		bool isTrigger = false;

	protected:
		static Wrappers::PhysicsEngine::JoltPhysicsEngine* physicEngine;
    };
}
