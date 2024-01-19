#include "Renderer/Uniform/RendererWindowUniform.hpp"

#include <array>

#include <ImGUI/imgui.h>

#include "Core/Debugging/Log.hpp"
#include "Core/App.hpp"

using namespace Renderer;
using namespace Renderer::Uniform;

void RendererWindowUniform::CreateDescriptorSetLayout(VkDevice& device, VkPhysicalDevice& physicalDevice)
{
    VkDescriptorSetLayoutBinding samplerLayoutBinding{};
    samplerLayoutBinding.binding = 0;
    samplerLayoutBinding.descriptorCount = 1;
    samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplerLayoutBinding.pImmutableSamplers = nullptr;
    samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    std::array<VkDescriptorSetLayoutBinding, 1> bindings = { samplerLayoutBinding };
    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<u32>(bindings.size());
    layoutInfo.pBindings = bindings.data();

    if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS)
    {
        LOG(DEBUG_LEVEL::LERROR, "Failed to create descriptor set layout!");
        throw std::runtime_error("Failed to create descriptor set layout!");
    }
}

void RendererWindowUniform::DestroyDescriptorSetLayout(VkDevice& device)
{
    vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
}

u64 RendererWindowUniform::GetVertexBufSize() const
{
    return 0;
}

u64 RendererWindowUniform::GetFragmentBufSize() const
{
    return 0;
}
