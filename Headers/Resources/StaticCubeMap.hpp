#pragma once

#include "Renderer/RendererTexture.hpp"

#include "Maths/Maths.hpp"

#include "CubeMap.hpp"

namespace Core
{
	class App;
}

namespace Resources
{
	class NAT_API StaticCubeMap : public CubeMap
	{
	public:
		StaticCubeMap();
		~StaticCubeMap() override = default;
		void DeleteData() override;
		void DeleteCubeMapData();
		void Write(Core::Serialization::Serializer& sr) override;
		void Load(Core::Serialization::Deserializer& dr) override;
		void WindowCreateResource(bool& open) override;

		void* cubeMapData = nullptr;
		bool isFloat = false;

		static StaticCubeMap* GetDebugCubeMap();
		static StaticCubeMap* GetDefaultCubeMap();

		Renderer::RendererTexture renderTexture;
		Renderer::RendererTexture cubeImageView;
		const Renderer::RendererImageView& GetImageView() const override;
		const Renderer::RendererTexture& GetRendererTexture() const override;
		const Renderer::RendererTexture& GetImGuiRendererTexture() const override;

	private:
		static StaticCubeMap* debugCubeMap;
		static StaticCubeMap* defaultCubeMap;

		void SetDebugCubeMap();
		void SetDefaultCubeMap();

		friend Core::App;
	};
}