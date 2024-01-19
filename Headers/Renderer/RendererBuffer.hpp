#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "Core/Types.hpp"
#include "Maths/Maths.hpp"
#include "RendererImageView.hpp"
#include "RendererTexture.hpp"

namespace Renderer
{
	class NAT_API RendererBuffer
	{
	public:
		RendererBuffer() = default;

		RendererBuffer(u32 width, u32 height, VkDevice& device, VkFormat format, bool transitLayout);

		~RendererBuffer() = default;

		void DeleteBuffer(VkDevice& device);

		RendererImageView& GetImageView() { return texture.imageView; }
		const RendererImageView& GetImageView() const { return texture.imageView; }
		Maths::IVec2 GetResolution() const { return resolution; }

		RendererTexture texture;
	private:
		Maths::IVec2 resolution;
	};

}