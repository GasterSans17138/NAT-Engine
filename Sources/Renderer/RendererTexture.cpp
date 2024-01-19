#include "Renderer/RendererTexture.hpp"

#include "Wrappers/ImageLoader.hpp"
#include "Renderer/RendererTextureSampler.hpp"
#include "Core/Debugging/Log.hpp"
#include "Core/App.hpp"
#include "Renderer/VulkanRenderer.hpp"

using namespace Maths;

namespace Renderer
{
    void RendererTexture::CreateGuiView(const RendererTextureSampler& sampler)
    {
        imageView.imGuiDS = Core::App::GetInstance()->GetInterfacing().CreateImageRef(imageView.imageView, sampler.GetSampler());
        isValid = true;
    }

    void RendererTexture::DeleteGuiView()
    {
        Core::App::GetInstance()->GetInterfacing().DeleteImageRef(imageView.imGuiDS);
        isValid = false;
    }
}