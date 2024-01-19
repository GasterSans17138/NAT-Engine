#pragma once

#include "Renderer/RendererDepthBuffer.hpp"
#include "Resources/Texture.hpp"

namespace Renderer
{
	class VulkanRenderer;
}

namespace LowRenderer
{
	class NAT_API DepthBuffer : public Resources::Texture
	{
	public:
		DepthBuffer();
		DepthBuffer(const Maths::IVec2 resolution);

		~DepthBuffer() override;

		void Write(Core::Serialization::Serializer& sr) override;
		void Load(Core::Serialization::Deserializer& dr) override;

		void DeleteData() override;
		const Renderer::RendererImageView& GetImageView() const override;
		const Renderer::RendererTexture& GetRendererTexture() const override;
		void Resize(Maths::IVec2 resolution);
		Maths::IVec2 GetResolution() const;
	private:
		Renderer::RendererDepthBuffer db;
		static Renderer::VulkanRenderer* renderer;
	};

}