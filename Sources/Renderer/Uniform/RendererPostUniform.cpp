#include "Renderer/Uniform/RendererPostUniform.hpp"

#include <array>

#include <ImGUI/imgui.h>

#include "Core/Debugging/Log.hpp"
#include "Core/App.hpp"

using namespace Renderer;
using namespace Renderer::Uniform;

void RendererPostUniform::CreateDescriptorSetLayout(VkDevice& device, VkPhysicalDevice& physicalDevice)
{
    VkDescriptorSetLayoutBinding fragLayoutBinding{};
    fragLayoutBinding.binding = 0;
    fragLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    fragLayoutBinding.descriptorCount = 1;
    fragLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragLayoutBinding.pImmutableSamplers = nullptr; // Optional

    VkDescriptorSetLayoutBinding colorLayoutBinding{};
    colorLayoutBinding.binding = 2;
    colorLayoutBinding.descriptorCount = 1;
    colorLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    colorLayoutBinding.pImmutableSamplers = nullptr;
    colorLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutBinding glowLayoutBinding{};
    glowLayoutBinding.binding = 3;
    glowLayoutBinding.descriptorCount = 1;
    glowLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    glowLayoutBinding.pImmutableSamplers = nullptr;
    glowLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    std::array<VkDescriptorSetLayoutBinding, 3> bindings = { fragLayoutBinding, colorLayoutBinding, glowLayoutBinding };
    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<u32>(bindings.size());
    layoutInfo.pBindings = bindings.data();

    if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS)
    {
        LOG(DEBUG_LEVEL::LERROR, "Failed to create descriptor set layout!");
        throw std::runtime_error("Failed to create descriptor set layout!");
    }

    VkPhysicalDeviceProperties p;
    vkGetPhysicalDeviceProperties(physicalDevice, &p);
    u64 size = p.limits.minUniformBufferOffsetAlignment;
    u64 modulo = GetTotalSize() % size;
    if (modulo == 0)
    {
        totalOffset = GetTotalSize();
    }
    else
    {
        totalOffset = GetTotalSize() + size - modulo;
    }
}

void RendererPostUniform::DestroyDescriptorSetLayout(VkDevice& device)
{
    vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
}

u64 RendererPostUniform::GetVertexBufSize() const
{
    return 0;
}

u64 RendererPostUniform::GetFragmentBufSize() const
{
    return sizeof(PostFragmentUniform);
}
