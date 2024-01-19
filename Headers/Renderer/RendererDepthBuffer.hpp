#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "Core/Types.hpp"
#include "Maths/Maths.hpp"
#include "RendererImageView.hpp"
#include "RendererTexture.hpp"

namespace Renderer
{
	class NAT_API RendererDepthBuffer
	{
	public:
		RendererDepthBuffer() = default;

		RendererDepthBuffer(u32 width, u32 height, VkDevice& device, bool transitLayout);

		~RendererDepthBuffer() = default;

		void DeleteDepthBuffer(VkDevice& device);

		RendererImageView& GetDepthImageView() { return texture.imageView; }
		const RendererImageView& GetDepthImageView() const { return texture.imageView; }
		Maths::IVec2 GetResolution() const { return resolution; }

		RendererTexture texture;
	private:
		Maths::IVec2 resolution;
	};

}