#pragma once

#include "PostProcessEffect.hpp"

namespace LowRenderer::PostProcess
{
	class NAT_API GammaCorrectionPostProcess : public PostProcessEffect
	{
	public:
		GammaCorrectionPostProcess();

		~GammaCorrectionPostProcess() override = default;

		void ApplyPass(Renderer::VulkanRenderer* renderer, LowRenderer::FrameBuffer* fb) override;

		void Configure() override;

		void FillBuffer(void* dest) const override;

		const char* GetEffectName() const override { return "Gamma Correction"; }

		EffectType GetType();
		void Serialize(Core::Serialization::Serializer& sr);
		void Deserialize(Core::Serialization::Deserializer& dr);

		PostProcessEffect* CreateCopy() const override;

	private:
		f32 exposure = 5.0f;
		f32 gamma = 0.417f;
	};

}