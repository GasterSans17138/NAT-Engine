#pragma once

#include "IRendererResource.hpp"
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <string>

namespace Renderer
{
	class NAT_API RendererShader : IRendererResource
	{
	public:
		RendererShader() = delete;
		RendererShader(VkShaderStageFlagBits type);
		~RendererShader() = default;
		bool LoadShader(const std::string& data, VkDevice & device);
		void DeleteShader(VkDevice & device);

		const VkShaderModule& GetModule() const { return shaderModule; }
		const VkPipelineShaderStageCreateInfo& GetStageInfo() const { return shaderStageInfo; }
	private:
		VkShaderModule shaderModule = {};
		VkPipelineShaderStageCreateInfo shaderStageInfo = {};
		VkShaderStageFlagBits shaderType;

	private:
	};

	class NAT_API FragmentRendererShader : public RendererShader
	{
	public:
		FragmentRendererShader();
		~FragmentRendererShader() = default;
	};

	class NAT_API VertexRendererShader : public RendererShader
	{
	public:
		VertexRendererShader();
		~VertexRendererShader() = default;
	};

	class NAT_API GeometryRendererShader : public RendererShader
	{
	public:
		GeometryRendererShader();
		~GeometryRendererShader() = default;
	};
}