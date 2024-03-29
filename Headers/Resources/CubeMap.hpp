#pragma once

#include "Renderer/RendererImageView.hpp"
#include "Renderer/RendererTexture.hpp"
#include "ResourceManager.hpp"

#include "IResource.hpp"

namespace Resources
{
	class NAT_API CubeMap : public IResource
	{
	public:
		virtual void Write(Core::Serialization::Serializer& sr) override;
		virtual void Load(Core::Serialization::Deserializer& dr) override;
		void UpdateMipLevel();
		virtual void DeleteData() override;
		void WindowCreateResource(bool& open) override;
		ObjectType GetType() override { return ObjectType::CubeMapType; }

		Maths::IVec2 resolution;
		u32 mipLevels = 0;

		virtual const Renderer::RendererImageView& GetImageView() const;
		virtual const Renderer::RendererTexture& GetRendererTexture() const;
		virtual const Renderer::RendererTexture& GetImGuiRendererTexture() const;

	};

}