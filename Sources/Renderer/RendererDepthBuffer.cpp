	#include "Renderer/RendererDepthBuffer.hpp"

#include "Core/App.hpp"

namespace Renderer
{
	RendererDepthBuffer::RendererDepthBuffer(u32 width, u32 height, VkDevice& device, bool transitLayout)
	{
		resolution = Maths::IVec2(Maths::Util::MaxI(width, 1), Maths::Util::MaxI(height, 1));
		VulkanRenderer& instance =  Core::App::GetInstance()->GetRenderer();
		VkFormat depthFormat = instance.FindDepthFormat();
		instance.CreateImage(resolution, 1, texture.textureImage, texture.textureImageMemory, depthFormat, VK_IMAGE_TILING_OPTIMAL, transitLayout ? VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT : VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		texture.imageView.imageView = instance.CreateImageView(texture.textureImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, 1);
		if (transitLayout) instance.TransitionImageLayout(texture.textureImage, depthFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, 1);
	}

	void RendererDepthBuffer::DeleteDepthBuffer(VkDevice& device)
	{
		vkDestroyImageView(device, texture.imageView.imageView, nullptr);
		vkDestroyImage(device, texture.textureImage, nullptr);
		vkFreeMemory(device, texture.textureImageMemory, nullptr);
	}

}
