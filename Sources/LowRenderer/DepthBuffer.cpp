#include "LowRenderer/DepthBuffer.hpp"

#include "Core/App.hpp"

using namespace LowRenderer;

Renderer::VulkanRenderer* DepthBuffer::renderer = nullptr;

DepthBuffer::DepthBuffer() : DepthBuffer(Maths::IVec2(256, 256))
{
}

LowRenderer::DepthBuffer::DepthBuffer(const Maths::IVec2 resolution)
{
	if (!renderer) renderer = &Core::App::GetInstance()->GetRenderer();
	renderer->CreateDepthBuffer(db, resolution);
}

DepthBuffer::~DepthBuffer()
{
}

void LowRenderer::DepthBuffer::Write(Core::Serialization::Serializer& sr)
{
	sr.Write(static_cast<u8>(Resources::ObjectType::DepthBufferType));
	Resources::Texture::Write(sr);
}

void LowRenderer::DepthBuffer::Load(Core::Serialization::Deserializer& dr)
{
	Resources::Texture::Load(dr);
}

void LowRenderer::DepthBuffer::DeleteData()
{
	renderer->DeleteDepthBuffer(db);
}

const Renderer::RendererImageView& LowRenderer::DepthBuffer::GetImageView() const
{
	return db.GetDepthImageView();
}

const Renderer::RendererTexture& LowRenderer::DepthBuffer::GetRendererTexture() const
{
	return db.texture;
}

void LowRenderer::DepthBuffer::Resize(Maths::IVec2 resolution)
{
	renderer->DeleteDepthBuffer(db);
	renderer->CreateDepthBuffer(db, resolution);
}

Maths::IVec2 LowRenderer::DepthBuffer::GetResolution() const
{
	return db.GetResolution();
}
