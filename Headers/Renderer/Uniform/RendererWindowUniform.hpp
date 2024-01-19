#pragma once

#include "Maths/Maths.hpp"
#include "RendererUniformObject.hpp"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace Renderer::Uniform
{
	class NAT_API RendererWindowUniform : public RendererUniformObject
	{
	public:
		RendererWindowUniform() = default;

		~RendererWindowUniform() override = default;

		void CreateDescriptorSetLayout(VkDevice& device, VkPhysicalDevice& physicalDevice) override;

		void DestroyDescriptorSetLayout(VkDevice& device) override;

		u64 GetVertexBufSize() const override;
		u64 GetFragmentBufSize() const override;

	private:
	};

}