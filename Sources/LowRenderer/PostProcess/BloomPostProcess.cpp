#include "LowRenderer/PostProcess/BloomPostProcess.hpp"

#include "Wrappers/Interfacing.hpp"
#include "Core/App.hpp"

using namespace LowRenderer::PostProcess;

BloomPostProcess::BloomPostProcess() : PostProcessEffect()
{
	shader = Core::App::GetInstance()->GetResources().Get<Resources::ShaderProgram>(0x24);
}

void BloomPostProcess::ApplyPass(Renderer::VulkanRenderer* renderer, LowRenderer::FrameBuffer* fb)
{
	vertical = false;
	if (fb)	bufferResolution = fb->GetResolution();
	else bufferResolution = renderer->GetMainLightBuffer().resolution;
	for (u32 i = 0; i < passCount; i++)
	{
		renderer->BeginPass(fb, LowRenderer::RenderPassType::POST);
		renderer->ApplyPostProcess(this);
		renderer->EndPass();
		if (fb)	fb->ToggleBuffer();
		else renderer->ToggleMainFB();
		vertical = !vertical;
	}
}

void BloomPostProcess::Configure()
{
	guiInterface->SliderInt("Number of passes", reinterpret_cast<int*>(&passCount), 2, 32);
}

void BloomPostProcess::FillBuffer(void* dest) const
{
	f32* buf = reinterpret_cast<f32*>(dest);
	if (vertical)
	{
		buf[0] = 0.0f;
		buf[1] = 1.0f;
		buf[2] = static_cast<f32>(bufferResolution.x);
		buf[3] = static_cast<f32>(bufferResolution.y);
	}
	else
	{
		buf[0] = 1.0f;
		buf[1] = 0.0f;
		buf[2] = static_cast<f32>(bufferResolution.x);
		buf[3] = static_cast<f32>(bufferResolution.y);
	}
}

EffectType BloomPostProcess::GetType()
{
	return EffectType::BLOOM;
}

void BloomPostProcess::Serialize(Core::Serialization::Serializer& sr)
{
	sr.Write(passCount);
}

void BloomPostProcess::Deserialize(Core::Serialization::Deserializer& dr)
{
	dr.Read(passCount);
}

PostProcessEffect* BloomPostProcess::CreateCopy() const
{
	return new BloomPostProcess(*this);
}
