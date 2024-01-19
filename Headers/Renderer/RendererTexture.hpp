#pragma once

#include "vulkan/vulkan_core.h"

#include "IRendererResource.hpp"
#include "RendererImageView.hpp"

namespace Renderer
{
	class RendererTextureSampler;

	class NAT_API RendererTexture : public IRendererResource
	{
	public:
		RendererTexture() = default;
		~RendererTexture() = default;

		VkImage textureImage = {};
		VkImageLayout layout = {};
		VkDeviceMemory textureImageMemory = {};
		RendererImageView imageView;

		void CreateGuiView(const RendererTextureSampler& sampler);
		void DeleteGuiView();
		bool IsvalidForRender() const { return isValid; }
	
	private:
		bool isValid = true;

		friend class VulkanRenderer;
	};
}