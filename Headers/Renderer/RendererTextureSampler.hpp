#pragma once

#include "IRendererResource.hpp"
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace Renderer
{
	class NAT_API RendererTextureSampler : IRendererResource
	{
	public:
		RendererTextureSampler() = default;

		RendererTextureSampler(VkDevice & device, VkFilter textureFilter = VK_FILTER_LINEAR, VkSamplerAddressMode textureWrap = VK_SAMPLER_ADDRESS_MODE_REPEAT, VkBorderColor border = VK_BORDER_COLOR_INT_OPAQUE_BLACK, bool enableCompare = false);

		~RendererTextureSampler();

		void DeleteTextureSampler(VkDevice & device);

		VkSampler& GetSampler() { return textureSampler; }
		const VkSampler& GetSampler() const { return textureSampler; }

	private:
		VkSampler textureSampler = {};
	
	private :
	};
}