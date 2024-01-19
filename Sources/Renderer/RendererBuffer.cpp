#include "Renderer/RendererBuffer.hpp"

#include "Core/App.hpp"

namespace Renderer
{
	RendererBuffer::RendererBuffer(u32 width, u32 height, VkDevice& device, VkFormat format, bool transitLayout)
	{
		resolution = Maths::IVec2(Maths::Util::MaxI(width, 1), Maths::Util::MaxI(height, 1));
		VulkanRenderer& instance = Core::App::GetInstance()->GetRenderer();
		instance.CreateImage(resolution, 1, texture.textureImage, texture.textureImageMemory, format, VK_IMAGE_TILING_OPTIMAL, transitLayout ? VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT : VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		texture.imageView.imageView = instance.CreateImageView(texture.textureImage, format, VK_IMAGE_ASPECT_COLOR_BIT, 1);
		if (transitLayout)
		{
			instance.TransitionImageLayout(texture.textureImage, format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 0);
		}
	}

	void RendererBuffer::DeleteBuffer(VkDevice& device)
	{
		vkDestroyImageView(device, texture.imageView.imageView, nullptr);
		vkDestroyImage(device, texture.textureImage, nullptr);
		vkFreeMemory(device, texture.textureImageMemory, nullptr);
	}

}
