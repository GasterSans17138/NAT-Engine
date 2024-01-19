#pragma once

#include "RendererUniformObject.hpp"
#include "Maths/Maths.hpp"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace Renderer::Uniform
{
	struct MainVertexUniform
	{
		Maths::Mat4 model;
		Maths::Mat4 mvp;
		Maths::Vec3 cameraPos;
	};

	struct MainFragmentUniform
	{
		Maths::Vec3 matAmbient = Maths::Vec3(1);
		f32 matShininess = 256.0f;
		f64 iTime = 0;
	};

	class NAT_API RendererMainUniform : public RendererUniformObject
	{
	public:
		RendererMainUniform() = default;

		~RendererMainUniform() = default;

		void CreateDescriptorSetLayout(VkDevice& device, VkPhysicalDevice& physicalDevice) override;

		void DestroyDescriptorSetLayout(VkDevice& device) override;

		u64 GetVertexBufSize() const override;
		u64 GetFragmentBufSize() const override;
	private:
	};

}