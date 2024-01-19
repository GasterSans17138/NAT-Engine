#pragma once

#include "Renderer/RendererFrameBuffer.hpp"
#include "Renderer/RendererDepthBuffer.hpp"
#include "Renderer/RendererBuffer.hpp"
#include "Resources/Texture.hpp"

namespace Renderer
{
	class VulkanRenderer;
}

namespace LowRenderer
{
	class NAT_API FrameBuffer : public Resources::Texture
	{
	public:
		FrameBuffer();

		~FrameBuffer() override;

		void CreateFrameBuffer(const Maths::IVec2 resolution = Maths::IVec2(256,256));
		void Write(Core::Serialization::Serializer& sr) override;
		void Load(Core::Serialization::Deserializer& dr) override;

		void DeleteData() override;
		const Renderer::RendererImageView& GetImageView() const override;
		const Renderer::RendererTexture& GetRendererTexture() const override;
		void Resize(Maths::IVec2 resolution);
		void Update();
		void UpdateClearColor();
		void ResetBuffer();
		void ToggleBuffer();
		Maths::IVec2 GetResolution() const;
		Maths::Vec4 ClearColor = Maths::Vec4(0,0,0,1);
	private:
		Renderer::RendererFrameBuffer fb;
		Renderer::RendererFrameBuffer lb[3]; // buffer #0 receive output of light pass, and buffer #1 and #2 are used by post process pass
		Renderer::RendererBuffer gb[3];
		Renderer::RendererBuffer nb;
		Renderer::RendererBuffer pb;
		Renderer::RendererDepthBuffer db;
		Renderer::RendererFrameBuffer old_fb;
		Renderer::RendererFrameBuffer old_lb[3];
		Renderer::RendererBuffer old_gb[3];
		Renderer::RendererBuffer old_nb;
		Renderer::RendererBuffer old_pb;
		Renderer::RendererDepthBuffer old_db;

		u8 resize = 0;
		u8 actualBuffer = 0;
		static Renderer::VulkanRenderer* renderer;

		friend Core::App;
		friend Renderer::VulkanRenderer;
	};

}