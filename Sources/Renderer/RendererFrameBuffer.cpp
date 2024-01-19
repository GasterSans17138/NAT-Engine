#include "Renderer/RendererFrameBuffer.hpp"

#include <stdexcept>

#include "Core/Debugging/Log.hpp"
#include "Core/App.hpp"

namespace Renderer
{
    RendererFrameBuffer::RendererFrameBuffer(const Maths::IVec2 resolutionIn, std::vector<VkImageView>& attachments, VkRenderPass& renderPass, VkDevice& device, VkImage imageIn, RendererImageView imageViewIn)
    {
        resolution = resolutionIn;
        rendererTex.textureImage = imageIn;
        rendererTex.imageView = imageViewIn;
        CreateFrameBuffer(attachments, renderPass, device);
    }

    RendererFrameBuffer::RendererFrameBuffer(const Maths::IVec2 resolutionIn, std::vector<VkImageView>& attachments, VkRenderPass& renderPass, VkDevice& device, VkFormat format, bool transitLayout)
    {
        resolution = Maths::IVec2(Maths::Util::MaxI(resolutionIn.x,1), Maths::Util::MaxI(resolutionIn.y,1));
        auto& renderer = Core::App::GetInstance()->GetRenderer();
        renderer.CreateImage(resolution, 1, rendererTex.textureImage, rendererTex.textureImageMemory, format, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        rendererTex.imageView.imageView = renderer.CreateImageView(rendererTex.textureImage, format, VK_IMAGE_ASPECT_COLOR_BIT, 1);
        CreateFrameBuffer(attachments, renderPass, device);
        if (transitLayout)
        {
            renderer.TransitionImageLayout(rendererTex.textureImage, format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 0);
        }
    }

    void RendererFrameBuffer::DeleteFrameBuffer(VkDevice& device, bool deleteImage)
    {
        vkDestroyFramebuffer(device, buffer, nullptr);
        if (rendererTex.imageView.imageView) vkDestroyImageView(device, rendererTex.imageView.imageView, nullptr);
        if (deleteImage)
        {
            vkDestroyImage(device, rendererTex.textureImage, nullptr);
            vkFreeMemory(device, rendererTex.textureImageMemory, nullptr);
        }
        if (rendererTex.imageView.imGuiDS) rendererTex.DeleteGuiView();
    }

    void RendererFrameBuffer::CreateFrameBuffer(std::vector<VkImageView> attachments, VkRenderPass& renderPass, VkDevice& device)
    {
        attachments.resize(attachments.size() + 1);
        for (u32 i = static_cast<u32>(attachments.size()-1); i > 0; i--)
        {
            attachments[i] = attachments[i - 1];
        }
        attachments[0] = rendererTex.imageView.imageView;

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderPass;
        framebufferInfo.attachmentCount = static_cast<u32>(attachments.size());
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width = resolution.x;
        framebufferInfo.height = resolution.y;
        framebufferInfo.layers = 1;
        if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &buffer) != VK_SUCCESS)
        {
            LOG(DEBUG_LEVEL::LERROR, "Failed to create framebuffer!");
            throw std::runtime_error("Failed to create framebuffer!");
        }
        clearValue.resize(attachments.size() + 1);
        for (u32 i = 0; i < attachments.size(); i++)
        {
            if (i == 1) clearValue[i].depthStencil = { 1.0f, 0 };
            else clearValue[i].color = { {0.0f, 0.0f, 0.0f, 0.0f} };
        }
    }

}
