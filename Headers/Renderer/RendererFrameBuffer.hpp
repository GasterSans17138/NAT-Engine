#pragma once

#include "RendererTexture.hpp"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>

#include "Maths/Maths.hpp"

namespace Renderer
{
	class NAT_API RendererFrameBuffer
	{
	public:
		RendererFrameBuffer() = default;
		RendererFrameBuffer(const Maths::IVec2 resolutionIn, std::vector<VkImageView>& attachments, VkRenderPass& renderPass, VkDevice& device, VkImage imageIn, RendererImageView imageViewIn);
		RendererFrameBuffer(const Maths::IVec2 resolutionIn, std::vector<VkImageView>& attachments, VkRenderPass& renderPass, VkDevice& device, VkFormat format = {}, bool transitLayout = false);

		~RendererFrameBuffer() = default;

		void DeleteFrameBuffer(VkDevice& device, bool deleteImage = true);
		Maths::IVec2 GetResolution() const { return resolution; }
		const std::vector<VkClearValue>& GetClearValue() const { return clearValue; }

		VkFramebuffer buffer = {};
		RendererTexture rendererTex;
		std::vector<VkClearValue> clearValue;
	private:
		Maths::IVec2 resolution;
		void CreateFrameBuffer(std::vector<VkImageView> attachments, VkRenderPass& renderPass, VkDevice& device);
	};

}