#pragma once

#include "IRendererResource.hpp"
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace Renderer
{
	class NAT_API RendererImageView : IRendererResource
	{
	public:
		RendererImageView() = default;

		~RendererImageView() = default;

		VkImageView imageView = {};
		VkDescriptorSet imGuiDS = {};

	private:
	};
}