#pragma once

#include "Maths/Maths.hpp"
#include "Core/Serialization/Serializer.hpp"
#include "Core/Serialization/Deserializer.hpp"

namespace Resources
{
	class ShaderProgram;
}

namespace Renderer
{
	class VulkanRenderer;
}

namespace Wrappers
{
	class Interfacing;
}

namespace LowRenderer
{
	class FrameBuffer;

	namespace PostProcess
	{
		enum class EffectType : u8
		{
			BLOOM = 0,
			BLUR,
			GAMMA_CORRECTION
		};

		class NAT_API PostProcessEffect
		{
		public:
			PostProcessEffect();

			virtual ~PostProcessEffect() = default;

			virtual void ApplyPass(Renderer::VulkanRenderer* renderer, LowRenderer::FrameBuffer* fb) = 0;

			virtual void Configure() = 0;

			virtual EffectType GetType() = 0;

			virtual void Serialize(Core::Serialization::Serializer& sr) = 0;
			virtual void Deserialize(Core::Serialization::Deserializer& dr) = 0;

			virtual PostProcessEffect* CreateCopy() const = 0;

			virtual const char* GetEffectName() const = 0;

			virtual void FillBuffer(void* dest) const = 0;
			Resources::ShaderProgram* GetShader() const;
		protected:
			Resources::ShaderProgram* shader = nullptr;
			static Wrappers::Interfacing* guiInterface;
		};
	}
}