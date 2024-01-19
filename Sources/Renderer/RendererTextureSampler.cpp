#include "Renderer/RendererTextureSampler.hpp"

#include "Core/Debugging/Log.hpp"
#include "Core/App.hpp"

namespace Renderer
{
	RendererTextureSampler::RendererTextureSampler(VkDevice& device, VkFilter textureFilter, VkSamplerAddressMode textureWrap, VkBorderColor border, bool enableCompare)
	{
		VkSamplerCreateInfo samplerInfo{};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = textureFilter;
		samplerInfo.minFilter = textureFilter;
		samplerInfo.addressModeU = textureWrap;
		samplerInfo.addressModeV = textureWrap;
		samplerInfo.addressModeW = textureWrap;
		samplerInfo.anisotropyEnable = VK_TRUE;

		VulkanRenderer& instance = Core::App::GetInstance()->GetRenderer();

		VkPhysicalDeviceProperties properties{};
		vkGetPhysicalDeviceProperties(instance.GetPhysicalDevice(), &properties);
		samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
		samplerInfo.borderColor = border;
		samplerInfo.unnormalizedCoordinates = VK_FALSE;
		samplerInfo.compareEnable = enableCompare;
		samplerInfo.compareOp = enableCompare ? VK_COMPARE_OP_LESS : VK_COMPARE_OP_ALWAYS;
		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerInfo.mipLodBias = 0.0f;
		samplerInfo.minLod = 0.0f;
		samplerInfo.maxLod = 64.0f;

		if (vkCreateSampler(device, &samplerInfo, nullptr, &textureSampler) != VK_SUCCESS)
		{
			LOG(DEBUG_LEVEL::LERROR, "Failed to create texture sampler!");
			throw std::runtime_error("failed to create texture sampler!");
		}
	}

	RendererTextureSampler::RendererTextureSampler::~RendererTextureSampler()
	{
	}

	void RendererTextureSampler::RendererTextureSampler::DeleteTextureSampler(VkDevice& device)
	{
		vkDestroySampler(device, textureSampler, nullptr);
	}

}

