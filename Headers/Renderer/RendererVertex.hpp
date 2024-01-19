#pragma once

#include <array>

#include "Maths/Maths.hpp"

#include "vulkan/vulkan_core.h"

#include "IRendererResource.hpp"

namespace Renderer
{
	//TODO : Vertex : This is not a IRendererResource. We can move this to general types !
	class NAT_API RendererVertex : IRendererResource
	{
	public:
		RendererVertex() = default;
		RendererVertex(Maths::Vec3 posIn, Maths::Vec3 colorIn, Maths::Vec2 uvIn, Maths::Vec3 normalIn, Maths::Vec3 tangentIn) :
			pos(posIn), color(colorIn), uv(uvIn), normal(normalIn), tangent(tangentIn) {}

		Maths::Vec3 pos;
		Maths::Vec3 color = Maths::Vec3(1);
		Maths::Vec2 uv;
		Maths::Vec3 normal;
		Maths::Vec3 tangent;

		~RendererVertex() = default;
		
		//TODO : vertex : Move this to renderer
		static VkVertexInputBindingDescription GetBindingDescription();
		static std::array<VkVertexInputAttributeDescription, 5> GetAttributeDescriptions();
	
	private :
	};
}
