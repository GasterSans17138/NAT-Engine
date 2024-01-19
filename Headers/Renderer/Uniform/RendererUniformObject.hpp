#pragma once

#include "Maths/Maths.hpp"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace Renderer::Uniform
{
	class NAT_API RendererUniformObject
	{
	public:
		RendererUniformObject() = default;

		virtual ~RendererUniformObject() = default;

		virtual void CreateDescriptorSetLayout(VkDevice& device, VkPhysicalDevice& physicalDevice) = 0;

		virtual void DestroyDescriptorSetLayout(VkDevice& device) = 0;

		VkDescriptorSetLayout& GetLayout() { return descriptorSetLayout; }
		const VkDescriptorSetLayout& GetLayout() const { return descriptorSetLayout; }

		u64 GetFragmentOffset() const;
		virtual u64 GetVertexBufSize() const = 0;
		virtual u64 GetFragmentBufSize() const = 0;
		u64 GetTotalSize() const;
		u64 GetTotalOffset() const;
	protected:
		VkDescriptorSetLayout descriptorSetLayout = {};
		u64 offset = 0;
		u64 totalOffset = 0;
	};

}