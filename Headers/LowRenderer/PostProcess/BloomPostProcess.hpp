#pragma once

#include "PostProcessEffect.hpp"

namespace LowRenderer::PostProcess
{
	class NAT_API BloomPostProcess : public PostProcessEffect
	{
	public:
		BloomPostProcess();

		~BloomPostProcess() override = default;

		void ApplyPass(Renderer::VulkanRenderer* renderer, LowRenderer::FrameBuffer* fb) override;

		void Configure() override;

		void FillBuffer(void* dest) const override;

		const char* GetEffectName() const override { return "Bloom"; }

		EffectType GetType();
		void Serialize(Core::Serialization::Serializer& sr);
		void Deserialize(Core::Serialization::Deserializer& dr);

		PostProcessEffect* CreateCopy() const override;

	private:
		u32 passCount = 6;
		Maths::IVec2 bufferResolution;
		bool vertical = false;
	};

}