#pragma once

#include "Renderer/RendererTexture.hpp"

#include "Maths/Maths.hpp"

#include "Texture.hpp"

namespace Core
{
	class App;
}

namespace Resources
{
	class NAT_API StaticTexture : public Texture
	{
	public:
		StaticTexture();
		~StaticTexture() override = default;
		void DeleteData() override;
		void DeleteTextureData();
		void Write(Core::Serialization::Serializer& sr) override;
		void Load(Core::Serialization::Deserializer& dr) override;
		void WindowCreateResource(bool& open) override;

		void* textureData = nullptr;
		bool isFloat = false;

		static StaticTexture* GetDebugTexture();
		static StaticTexture* GetDefaultTexture();
		static StaticTexture* GetDefaultNormal();
		
		Renderer::RendererTexture renderTexture;
		const Renderer::RendererImageView& GetImageView() const override;
		const Renderer::RendererTexture& GetRendererTexture() const override;

	private:
		static StaticTexture* debugTexture;
		static StaticTexture* defaultTexture;
		static StaticTexture* defaultNormal;

		void SetDebugTexture();
		void SetDefaultTexture();
		void SetDefaultNormal();

		friend Core::App;
	};
}