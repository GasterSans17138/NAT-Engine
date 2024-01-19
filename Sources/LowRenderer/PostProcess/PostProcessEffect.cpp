#include "LowRenderer/PostProcess/PostProcessEffect.hpp"

#include "Core/App.hpp"

using namespace LowRenderer::PostProcess;

Wrappers::Interfacing* PostProcessEffect::guiInterface = nullptr;

PostProcessEffect::PostProcessEffect()
{
	guiInterface = &Core::App::GetInstance()->GetInterfacing();
}

Resources::ShaderProgram* PostProcessEffect::GetShader() const
{
	return shader;
}