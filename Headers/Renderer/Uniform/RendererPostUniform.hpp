#pragma once

#include "Maths/Maths.hpp"
#include "RendererUniformObject.hpp"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace Renderer::Uniform
{
	struct PostFragmentUniform
	{
		Maths::Vec4 paramsA;
		Maths::Vec4 paramsB;
		Maths::Vec4 paramsC;
		u32 paramsD[4];
	};

	class NAT_API RendererPostUniform : public RendererUniformObject
	{
	public:
		RendererPostUniform() = default;

		~RendererPostUniform() override = default;

		void CreateDescriptorSetLayout(VkDevice& device, VkPhysicalDevice& physicalDevice) override;

		void DestroyDescriptorSetLayout(VkDevice& device) override;

		u64 GetVertexBufSize() const override;
		u64 GetFragmentBufSize() const override;

	private:
	};

}