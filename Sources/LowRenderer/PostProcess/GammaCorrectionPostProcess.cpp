#include "LowRenderer/PostProcess/GammaCorrectionPostProcess.hpp"

#include "Wrappers/Interfacing.hpp"
#include "Core/App.hpp"

using namespace LowRenderer::PostProcess;

GammaCorrectionPostProcess::GammaCorrectionPostProcess() : PostProcessEffect()
{
	shader = Core::App::GetInstance()->GetResources().Get<Resources::ShaderProgram>(0x22);
}

void GammaCorrectionPostProcess::ApplyPass(Renderer::VulkanRenderer* renderer, LowRenderer::FrameBuffer* fb)
{
	renderer->BeginPass(fb, LowRenderer::RenderPassType::POST);
	renderer->ApplyPostProcess(this);
	renderer->EndPass();
	if (fb)	fb->ToggleBuffer();
	else renderer->ToggleMainFB();
}

void GammaCorrectionPostProcess::Configure()
{
	guiInterface->SliderFloat("Exposure", &exposure, 0.001f, 65536.0f, true);
	guiInterface->SliderFloat("Gamma", &gamma, 0.001f, 65536.0f, true);
}

void GammaCorrectionPostProcess::FillBuffer(void* dest) const
{
	f32* buf = reinterpret_cast<f32*>(dest);
	buf[0] = exposure;
	buf[1] = gamma;
}

EffectType GammaCorrectionPostProcess::GetType()
{
	return EffectType::GAMMA_CORRECTION;
}

void GammaCorrectionPostProcess::Serialize(Core::Serialization::Serializer& sr)
{
	sr.Write(exposure);
	sr.Write(gamma);
}

void GammaCorrectionPostProcess::Deserialize(Core::Serialization::Deserializer& dr)
{
	dr.Read(exposure);
	dr.Read(gamma);
}

PostProcessEffect* GammaCorrectionPostProcess::CreateCopy() const
{
	return new GammaCorrectionPostProcess(*this);
}
