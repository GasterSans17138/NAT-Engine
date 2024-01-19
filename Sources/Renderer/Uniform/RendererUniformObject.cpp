#include "Renderer/Uniform/RendererUniformObject.hpp"

#include <array>

#include <ImGUI/imgui.h>

#include "Core/Debugging/Log.hpp"
#include "Core/App.hpp"

using namespace Renderer::Uniform;

void RendererUniformObject::DestroyDescriptorSetLayout(VkDevice& device)
{
    vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
}

u64 RendererUniformObject::GetFragmentOffset() const
{
    return offset;
}

u64 RendererUniformObject::GetTotalSize() const
{
    return offset + GetFragmentBufSize();
}

u64 RendererUniformObject::GetTotalOffset() const
{
    return totalOffset;
}