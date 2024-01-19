#include "LowRenderer/PostProcess/BlurPostProcess.hpp"

#include "Wrappers/Interfacing.hpp"
#include "Core/App.hpp"

using namespace LowRenderer::PostProcess;

BlurPostProcess::BlurPostProcess() : PostProcessEffect()
{
	shader = Core::App::GetInstance()->GetResources().Get<Resources::ShaderProgram>(0x26);
}

void BlurPostProcess::ApplyPass(Renderer::VulkanRenderer* renderer, LowRenderer::FrameBuffer* fb)
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

void BlurPostProcess::Configure()
{
	guiInterface->SliderInt("Number of passes", reinterpret_cast<int*>(&passCount), 2, 32);
}

void BlurPostProcess::FillBuffer(void* dest) const
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

EffectType BlurPostProcess::GetType()
{
	return EffectType::BLUR;
}

void BlurPostProcess::Serialize(Core::Serialization::Serializer& sr)
{
	sr.Write(passCount);
}

void BlurPostProcess::Deserialize(Core::Serialization::Deserializer& dr)
{
	dr.Read(passCount);
}

PostProcessEffect* BlurPostProcess::CreateCopy() const
{
	return new BlurPostProcess(*this);
}
