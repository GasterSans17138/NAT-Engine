#pragma once

#include "IRendererResource.hpp"

namespace Renderer
{
	class NAT_API RendererPipeline : IRendererResource
	{
	public:
		RendererPipeline() = default;
		~RendererPipeline() = default;

		VkPipelineLayout pipelineLayout = {};
		VkPipeline pipeline = {};
	private:
	};
}
