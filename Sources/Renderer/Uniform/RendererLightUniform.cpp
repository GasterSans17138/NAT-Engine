#include "Renderer/Uniform/RendererLightUniform.hpp"

#include <array>

#include <ImGUI/imgui.h>

#include "Core/Debugging/Log.hpp"
#include "Core/App.hpp"

using namespace Renderer;
using namespace Renderer::Uniform;

void RendererLightUniform::CreateDescriptorSetLayout(VkDevice& device, VkPhysicalDevice& physicalDevice)
{
    VkDescriptorSetLayoutBinding fragLayoutBinding{};
    fragLayoutBinding.binding = 0;
    fragLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    fragLayoutBinding.descriptorCount = 1;
    fragLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragLayoutBinding.pImmutableSamplers = nullptr; // Optional

    VkDescriptorSetLayoutBinding albedoLayoutBinding{};
    albedoLayoutBinding.binding = 2;
    albedoLayoutBinding.descriptorCount = 1;
    albedoLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    albedoLayoutBinding.pImmutableSamplers = nullptr;
    albedoLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutBinding normalLayoutBinding{};
    normalLayoutBinding.binding = 3;
    normalLayoutBinding.descriptorCount = 1;
    normalLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    normalLayoutBinding.pImmutableSamplers = nullptr;
    normalLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutBinding posLayoutBinding{};
    posLayoutBinding.binding = 4;
    posLayoutBinding.descriptorCount = 1;
    posLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    posLayoutBinding.pImmutableSamplers = nullptr;
    posLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    std::array<VkDescriptorSetLayoutBinding, 4> bindings = { fragLayoutBinding, albedoLayoutBinding, normalLayoutBinding, posLayoutBinding };
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

void RendererLightUniform::DestroyDescriptorSetLayout(VkDevice& device)
{
    vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
}

u64 RendererLightUniform::GetVertexBufSize() const
{
    return 0;
}

u64 RendererLightUniform::GetFragmentBufSize() const
{
    return sizeof(LightFragmentUniform);
}
