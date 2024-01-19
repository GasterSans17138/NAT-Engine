#pragma once

#include "Core/Types.hpp"
#include "RendererShader.hpp"
#include "RendererPipeline.hpp"

namespace Renderer
{
	class VulkanRenderer;

	class NAT_API RendererShaderProgram : IRendererResource
	{
	public:
		RendererShaderProgram() = default;
		RendererShaderProgram(const VertexRendererShader* vertex, const FragmentRendererShader* fragment, const GeometryRendererShader* geometry = nullptr);
		~RendererShaderProgram() = default;
		void DeleteShaderProgram(VkDevice& device);
		
		RendererPipeline pipeline = {};

		const VertexRendererShader* vertex = nullptr;
		const FragmentRendererShader* fragment = nullptr;
		const GeometryRendererShader* geom = nullptr;

	private:
	};
}