#pragma once

#include "vulkan/vulkan_core.h"

#include "IRendererResource.hpp"

namespace Renderer
{
	//TODO : Specific file for custom renderer types (renderer/utils.hpp ? renderer/types.hpp ?)

	struct VertexBuffer
	{
		VkBuffer handle			= VK_NULL_HANDLE;
		VkDeviceMemory memory	= VK_NULL_HANDLE;
	};

	typedef VertexBuffer IndiceBuffer;

	//

	class NAT_API RendererMesh : public IRendererResource
	{
	public:
		VertexBuffer vertexBuffer = {};
		IndiceBuffer indexBuffer = {};
	private :
	};
}