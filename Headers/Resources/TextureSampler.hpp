#pragma once

#include "IResource.hpp"

#include "Renderer/RendererTextureSampler.hpp"
#include "ResourceManager.hpp"

namespace Core
{
	class App;
}

namespace Resources
{
	class NAT_API TextureSampler : public IResource
	{
	public:
		TextureSampler();

		~TextureSampler() override;

		void DeleteData() override;

		void Write(Core::Serialization::Serializer& sr) override;
		void Load(Core::Serialization::Deserializer& dr) override;
		void WindowCreateResource(bool& open) override;
		ObjectType GetType() override { return ObjectType::TextureSamplerType; }

		static TextureSampler* GetDefaultSampler();

		Renderer::RendererTextureSampler renderSampler;
		bool wrap = true;
		bool linear = true;
	private:
		static TextureSampler* defaultSampler;
		void SetDefaultSampler();
		friend Core::App;
		TextureSampler* newTextureS = nullptr;
		bool prevbool = false;
	};

}