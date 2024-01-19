#include "Core/Debugging/Assert.hpp"

#include "Wrappers/PhysicsEngine/BroadPhaseLayer.hpp"
#include "Core/Scene/Components/IPhysicalComponent.hpp"

namespace Wrappers::PhysicsEngine
{
	BroadPhaseLayerImpl::BroadPhaseLayerImpl()
	{
		mBroadPhaseLayer[0] = BroadPhaseLayers::DEFAULT;
		mBroadPhaseLayer[1] = BroadPhaseLayers::TRIGGER;
	}

	u32 BroadPhaseLayerImpl::GetNumBroadPhaseLayers() const
	{
		return BroadPhaseLayers::LAYER_COUNT;
	}

	JPH::BroadPhaseLayer BroadPhaseLayerImpl::GetBroadPhaseLayer(JPH::ObjectLayer inLayer) const
	{
		return mBroadPhaseLayer[0];
	}

#if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)
	const char* BroadPhaseLayerImpl::GetBroadPhaseLayerName(JPH::BroadPhaseLayer inLayer) const
	{
		switch ((JPH::BroadPhaseLayer::Type)inLayer)
		{
		case (JPH::BroadPhaseLayer::Type)BroadPhaseLayers::DEFAULT:	return "DEFAULT";
		case (JPH::BroadPhaseLayer::Type)BroadPhaseLayers::TRIGGER:		return "TRIGGER";
		default:										 Assert(false); return "INVALID";
		}
	}
#endif // JPH_EXTERNAL_PROFILE || JPH_PROFILE_ENABLED
}