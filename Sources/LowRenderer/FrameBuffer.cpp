#include "LowRenderer/FrameBuffer.hpp"

#include "Core/App.hpp"

using namespace LowRenderer;

Renderer::VulkanRenderer* FrameBuffer::renderer = nullptr;

FrameBuffer::FrameBuffer()
{
}

void FrameBuffer::CreateFrameBuffer(const Maths::IVec2 resolutionIn)
{
	resolution = resolutionIn;
	if (!renderer) renderer = &Core::App::GetInstance()->GetRenderer();
	renderer->CreateDepthBuffer(db, resolution);
	renderer->CreateBuffer(nb, resolution);
	renderer->CreateBuffer(pb, resolution);
	std::vector<Renderer::RendererImageView> vec;
	vec.push_back(db.GetDepthImageView());
	vec.push_back(nb.GetImageView());
	vec.push_back(pb.GetImageView());
	renderer->CreateFrameBuffer(fb, vec, resolution, ClearColor);
	for (u8 i = 0; i < 3; i++)
	{
		renderer->CreateBuffer(gb[i], resolution, true);
		vec.clear();
		vec.push_back(gb[i].GetImageView());
		renderer->CreateFrameBuffer(lb[i], vec, resolution, Maths::Vec4(), VK_FORMAT_R32G32B32A32_SFLOAT, Resources::ShaderVariant::Light);
	}
}

FrameBuffer::~FrameBuffer()
{
}

void FrameBuffer::Write(Core::Serialization::Serializer& sr)
{
	sr.Write(static_cast<u8>(Resources::ObjectType::FrameBufferType));
	Resources::Texture::Write(sr);
	sr.Write(ClearColor);
}

void FrameBuffer::Load(Core::Serialization::Deserializer& dr)
{
	Resources::Texture::Load(dr);
	dr.Read(ClearColor);
	CreateFrameBuffer(resolution);
	isLoaded = true;
}

void FrameBuffer::DeleteData()
{
	renderer->DeleteFrameBuffer(fb);
	renderer->DeleteBuffer(nb);
	renderer->DeleteBuffer(pb);
	for (u8 i = 0; i < 3; i++)
	{
		renderer->DeleteFrameBuffer(lb[i]);
		renderer->DeleteBuffer(gb[i]);
	}
	renderer->DeleteDepthBuffer(db);
	if (resize)
	{
		renderer->DeleteFrameBuffer(old_fb);
		renderer->DeleteDepthBuffer(old_db);
		renderer->DeleteBuffer(old_nb);
		renderer->DeleteBuffer(old_pb);
		for (u8 i = 0; i < 3; i++)
		{
			renderer->DeleteFrameBuffer(old_lb[i]);
			renderer->DeleteBuffer(old_gb[i]);
		}
	}
}

const Renderer::RendererImageView& FrameBuffer::GetImageView() const
{
	return lb[actualBuffer].rendererTex.imageView;
}

const Renderer::RendererTexture& FrameBuffer::GetRendererTexture() const
{
	return lb[actualBuffer].rendererTex;
}

void FrameBuffer::Resize(Maths::IVec2 resolution)
{
	old_fb = fb;
	old_db = db;
	old_nb = nb;
	old_pb = pb;
	renderer->CreateDepthBuffer(db, resolution);
	renderer->CreateBuffer(nb, resolution);
	renderer->CreateBuffer(pb, resolution);
	std::vector<Renderer::RendererImageView> vec;
	vec.push_back(db.GetDepthImageView());
	vec.push_back(nb.GetImageView());
	vec.push_back(pb.GetImageView());
	renderer->CreateFrameBuffer(fb, vec, resolution, ClearColor);
	for (u8 i = 0; i < 3; i++)
	{
		old_gb[i] = gb[i];
		old_lb[i] = lb[i];
		renderer->CreateBuffer(gb[i], resolution, true);
		vec.clear();
		vec.push_back(gb[i].GetImageView());
		renderer->CreateFrameBuffer(lb[i], vec, resolution, Maths::Vec4(), VK_FORMAT_R32G32B32A32_SFLOAT, Resources::ShaderVariant::Light);
	}
	
	resize = 5;
}

void LowRenderer::FrameBuffer::Update()
{
	if (resize == 1)
	{
		renderer->DeleteFrameBuffer(old_fb);
		renderer->DeleteDepthBuffer(old_db);
		renderer->DeleteBuffer(old_nb);
		for (u8 i = 0; i < 3; i++)
		{
			renderer->DeleteFrameBuffer(old_lb[i]);
			renderer->DeleteBuffer(old_gb[i]);
		}
		renderer->DeleteBuffer(old_pb);
	}
	if (resize) resize--;
}

void LowRenderer::FrameBuffer::UpdateClearColor()
{
	if (fb.clearValue.size()) fb.clearValue[0].color = {ClearColor.x, ClearColor.y, ClearColor.z, ClearColor.w};
}

void LowRenderer::FrameBuffer::ResetBuffer()
{
	actualBuffer = 0;
}

void LowRenderer::FrameBuffer::ToggleBuffer()
{
	if (actualBuffer < 2) ++actualBuffer;
	else actualBuffer = 1;
}

Maths::IVec2 FrameBuffer::GetResolution() const
{
	return lb[actualBuffer].GetResolution();
}
