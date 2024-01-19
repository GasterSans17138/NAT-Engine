#pragma once

#include "Maths/Maths.hpp"
#include "RendererUniformObject.hpp"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define MAX_DIR_LIGHT 8
#define MAX_POINT_LIGHT 64
#define MAX_SPOT_LIGHT 64

namespace Renderer::Uniform
{

	struct DirectionalLight
	{
		Maths::Vec3 Direction;
		f32 Shininess;
		Maths::Vec4 Ambient;
		Maths::Vec4 Diffuse;
		Maths::Vec4 Specular;
	};

	struct PointLight
	{
		Maths::Vec3 Position;
		f32 Shininess;
		Maths::Vec3 Ambient;
		f32 Radius;
		Maths::Vec3 Diffuse;
		f32 Linear;
		Maths::Vec3 Specular;
		f32 Quadratic;
	};

	struct SpotLight
	{
		Maths::Vec3 Position;
		float Shininess;
		Maths::Vec3 Ambient;
		float Radius;
		Maths::Vec3 Diffuse;
		float Linear;
		Maths::Vec3 Specular;
		float Quadratic;
		Maths::Vec4 Direction;
		Maths::Vec2 Angles;
		f32 padding[2];
	};

	struct LightFragmentUniform
	{
		DirectionalLight dlights[MAX_DIR_LIGHT];
		PointLight plights[MAX_POINT_LIGHT];
		SpotLight slights[MAX_SPOT_LIGHT];
		Maths::Vec3 viewPos;
		u32 dCount;
		u32 pCount;
		u32 sCount;
	};

	class NAT_API RendererLightUniform : public RendererUniformObject
	{
	public:
		RendererLightUniform() = default;

		~RendererLightUniform() override = default;

		void CreateDescriptorSetLayout(VkDevice& device, VkPhysicalDevice& physicalDevice) override;

		void DestroyDescriptorSetLayout(VkDevice& device) override;

		u64 GetVertexBufSize() const override;
		u64 GetFragmentBufSize() const override;

	private:
	};

}