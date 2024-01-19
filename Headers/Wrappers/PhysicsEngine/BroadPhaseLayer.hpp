#pragma once

#include "Core/Types.hpp"

#include "Jolt/Jolt.h"
#include "Jolt/Physics/Collision/BroadPhase/BroadPhaseLayer.h"

namespace Wrappers::PhysicsEngine
{
	namespace BroadPhaseLayers
	{
		static constexpr JPH::BroadPhaseLayer DEFAULT(0);
		static constexpr JPH::BroadPhaseLayer TRIGGER(1);
		static constexpr u32 LAYER_COUNT = 2;
	}

	class NAT_API BroadPhaseLayerImpl : public JPH::BroadPhaseLayerInterface
	{
	public:
		BroadPhaseLayerImpl();
		~BroadPhaseLayerImpl() = default;

		virtual u32 GetNumBroadPhaseLayers() const override;

		virtual JPH::BroadPhaseLayer GetBroadPhaseLayer(JPH::ObjectLayer inLayer) const override;

#if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)
		virtual const char* GetBroadPhaseLayerName(JPH::BroadPhaseLayer inLayer) const override;
#endif // JPH_EXTERNAL_PROFILE || JPH_PROFILE_ENABLED

	private:
		JPH::BroadPhaseLayer mBroadPhaseLayer[BroadPhaseLayers::LAYER_COUNT];
	};
}