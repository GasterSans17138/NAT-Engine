#include "Renderer/RendererShaderProgram.hpp"

namespace Renderer 
{
	Renderer::RendererShaderProgram::RendererShaderProgram(const VertexRendererShader* vertexIn, const FragmentRendererShader* fragmentIn, const GeometryRendererShader* geometryIn) :
		vertex(vertexIn), fragment(fragmentIn), geom(geometryIn)
	{
	}

	void RendererShaderProgram::DeleteShaderProgram(VkDevice& device)
	{
	}
};