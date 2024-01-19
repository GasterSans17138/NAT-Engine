#include "Renderer/RendererShader.hpp"

#include <iostream>
#include <fstream>

#include "Core/Debugging/Log.hpp"
#include "Core/FileManager.hpp"

#pragma warning(disable : 26812)

namespace Renderer
{
	RendererShader::RendererShader(VkShaderStageFlagBits type)
	{
		shaderType = type;
	}

	bool RendererShader::LoadShader(const std::string& data, VkDevice& device)
	{
		if (data.empty()) return false;

		VkShaderModuleCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = data.size();
		createInfo.pCode = reinterpret_cast<const u32*>(data.c_str());
		if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
		{
			LOG(DEBUG_LEVEL::LERROR, "Failed to create shader module!");
			return false;
		}
		shaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shaderStageInfo.stage = shaderType;
		shaderStageInfo.module = shaderModule;
		shaderStageInfo.pName = "main";
		return true;
	}

	void RendererShader::DeleteShader(VkDevice& device)
	{
		vkDestroyShaderModule(device, shaderModule, nullptr);
		shaderModule = VK_NULL_HANDLE;
	}

	FragmentRendererShader::FragmentRendererShader() : RendererShader(VK_SHADER_STAGE_FRAGMENT_BIT)
	{
	}

	VertexRendererShader::VertexRendererShader() : RendererShader(VK_SHADER_STAGE_VERTEX_BIT)
	{
	}

	GeometryRendererShader::GeometryRendererShader() : RendererShader(VK_SHADER_STAGE_GEOMETRY_BIT)
	{
	}
}