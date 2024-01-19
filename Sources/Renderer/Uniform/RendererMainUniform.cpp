#include "Renderer/Uniform/RendererMainUniform.hpp"

#include <array>

#include <ImGUI/imgui.h>

#include "Core/Debugging/Log.hpp"
#include "Core/App.hpp"

using namespace Renderer;
using namespace Renderer::Uniform;

void RendererMainUniform::CreateDescriptorSetLayout(VkDevice& device, VkPhysicalDevice& physicalDevice)
{
    VkDescriptorSetLayoutBinding vertLayoutBinding{};
    vertLayoutBinding.binding = 0;
    vertLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    vertLayoutBinding.descriptorCount = 1;
    vertLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    vertLayoutBinding.pImmutableSamplers = nullptr; // Optional

    VkDescriptorSetLayoutBinding fragLayoutBinding{};
    fragLayoutBinding.binding = 1;
    fragLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    fragLayoutBinding.descriptorCount = 1;
    fragLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragLayoutBinding.pImmutableSamplers = nullptr; // Optional

    VkDescriptorSetLayoutBinding samplerLayoutBinding{};
    samplerLayoutBinding.binding = 2;
    samplerLayoutBinding.descriptorCount = 1;
    samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplerLayoutBinding.pImmutableSamplers = nullptr;
    samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutBinding normalLayoutBinding{};
    normalLayoutBinding.binding = 3;
    normalLayoutBinding.descriptorCount = 1;
    normalLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    normalLayoutBinding.pImmutableSamplers = nullptr;
    normalLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutBinding heightLayoutBinding{};
    heightLayoutBinding.binding = 4;
    heightLayoutBinding.descriptorCount = 1;
    heightLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    heightLayoutBinding.pImmutableSamplers = nullptr;
    heightLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    std::array<VkDescriptorSetLayoutBinding, 5> bindings = { vertLayoutBinding, fragLayoutBinding, samplerLayoutBinding, normalLayoutBinding, heightLayoutBinding };
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
    u64 modulo = sizeof(MainVertexUniform) % size;
    if (modulo == 0)
    {
        offset = sizeof(MainVertexUniform);
    }
    else
    {
        offset = sizeof(MainVertexUniform) + size - modulo;
    }
    modulo = GetTotalSize() % size;
    if (modulo == 0)
    {
        totalOffset = GetTotalSize();
    }
    else
    {
        totalOffset = GetTotalSize() + size - modulo;
    }
}

void RendererMainUniform::DestroyDescriptorSetLayout(VkDevice& device)
{
    vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
}

u64 RendererMainUniform::GetVertexBufSize() const
{
    return sizeof(MainVertexUniform);
}

u64 RendererMainUniform::GetFragmentBufSize() const
{
    return sizeof(MainFragmentUniform);
}
