#include "Resources/Texture.hpp"

#include "Core/Serialization/Serializer.hpp"
#include "Renderer/VulkanRenderer.hpp"

using namespace Resources;
using namespace Core::Serialization;

void Texture::Write(Serializer& sr)
{
	IResource::Write(sr);
	sr.Write(resolution.x); // resource data
	sr.Write(resolution.y);
}

void Texture::Load(Deserializer& dr)
{
	IResource::Load(dr);
	dr.Read(resolution.x);
	dr.Read(resolution.y);
}

void Texture::WindowCreateResource(bool& open)
{
	
}

void Texture::UpdateMipLevel()
{
	mipLevels = static_cast<u32>(floorf(std::log2f(static_cast<f32>(std::max(resolution.x, resolution.y))))) + 1;
}

void Texture::DeleteData()
{
}

const Renderer::RendererImageView& Texture::GetImageView() const
{
	Assert(0);
	return static_cast<Renderer::RendererImageView&>(*static_cast<Renderer::RendererImageView*>(nullptr));
}

const Renderer::RendererTexture& Texture::GetRendererTexture() const
{
	Assert(0);
	return static_cast<Renderer::RendererTexture&>(*static_cast<Renderer::RendererTexture*>(nullptr));
}
