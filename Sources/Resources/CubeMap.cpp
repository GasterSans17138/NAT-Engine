#include "Resources/CubeMap.hpp"

#include "Core/Serialization/Serializer.hpp"
#include "Renderer/VulkanRenderer.hpp"

using namespace Resources;
using namespace Core::Serialization;

void CubeMap::Write(Serializer& sr)
{
	IResource::Write(sr);
	sr.Write(resolution.x); // resource data
	sr.Write(resolution.y);
}

void CubeMap::Load(Deserializer& dr)
{
	IResource::Load(dr);
	dr.Read(resolution.x);
	dr.Read(resolution.y);
}

void CubeMap::UpdateMipLevel()
{
	mipLevels = static_cast<u32>(floorf(std::log2f(static_cast<f32>(std::max(resolution.x, resolution.y))))) + 1;
}

void CubeMap::DeleteData()
{
}

const Renderer::RendererImageView& CubeMap::GetImageView() const
{
	Assert(0);
	return static_cast<Renderer::RendererImageView&>(*static_cast<Renderer::RendererImageView*>(nullptr));
}

const Renderer::RendererTexture& CubeMap::GetRendererTexture() const
{
	Assert(0);
	return static_cast<Renderer::RendererTexture&>(*static_cast<Renderer::RendererTexture*>(nullptr));
}

const Renderer::RendererTexture& CubeMap::GetImGuiRendererTexture() const
{
	Assert(0);
	return static_cast<Renderer::RendererTexture&>(*static_cast<Renderer::RendererTexture*>(nullptr));
}

void CubeMap::WindowCreateResource(bool& open)
{
	
}