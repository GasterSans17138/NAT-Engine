#pragma once

#include <array>

#include "IResource.hpp"
#include "Renderer/RendererShaderProgram.hpp"
#include "LowRenderer/RenderPassType.hpp"
#include "ResourceManager.hpp"
#include "Shader.hpp"

namespace Core
{
	class App;
}

namespace Renderer
{
	class VulkanRenderer;
}

namespace Resources
{
	enum class NAT_API ShaderVariant : u8
	{
		Default = 0,
		Shadow,
		Cube,
		ShadowCube,
		Object,
		Halo,
		Light,
		PostProcess,
		WireFrame,
		Depth,
		SkyBox,
		Window,
		Line,
	};

	constexpr std::array<const char*, 11> shaderVariantStr = {
		"Default",
		"Shadow",
		"Cube",
		"ShadowCube",
		"Object",
		"Halo",
		"Light",
		"PostProcess",
		"WireFrame",
		"Depth",
		"SkyBox",
	};

	class NAT_API ShaderProgram : public IResource
	{
	public:
		ShaderProgram() = default;
		~ShaderProgram() override;

		void DeleteData() override;

		void CreateShaderProgram(VertexShader* vertex, FragmentShader* fragment, GeometryShader* geometry = nullptr, ShaderVariant variant = ShaderVariant::Default);
		const ShaderProgram* GetShader(ShaderVariant variant = ShaderVariant::Default) const;
		ShaderVariant GetShaderType() const { return type; }
		static ShaderProgram* GetDefaultShader();
		ObjectType GetType() override { return ObjectType::ShaderProgramType; }

	private:
		void AddShaderVariant(ShaderProgram* variant, ShaderVariant type);
		void AddShadowVariant(ShaderProgram* variant);
		void AddCubeVariant(ShaderProgram* variant);
		void AddShadowCubeVariant(ShaderProgram* variant);
		void AddObjectVariant(ShaderProgram* variant);
		void AddHaloVariant(ShaderProgram* variant);
		void SetDefaultShader();
		void Write(Core::Serialization::Serializer& sr) override;
		void Load(Core::Serialization::Deserializer& dr) override;
		LowRenderer::RenderPassType GetRenderPass();
		bool ShowEditWindow() override;
		void WindowCreateResource(bool &open) override;
		void ResetShaderprog(ShaderProgram* shaderprog);
		ShaderProgram* newShaderprog = nullptr;
		bool prevbool = false;

		VertexShader* vertex = nullptr;
		FragmentShader* fragment = nullptr;
		GeometryShader* geometry = nullptr;
		Renderer::RendererShaderProgram program;
		ShaderVariant type = ShaderVariant::Default;
		ShaderProgram* shadowVariant = nullptr;
		ShaderProgram* cubeVariant = nullptr;
		ShaderProgram* shadowCubeVariant = nullptr;
		ShaderProgram* objectVariant = nullptr;
		ShaderProgram* haloVariant = nullptr;

		static ShaderProgram* defaultShaderProgram;
		friend Core::App;
		friend Renderer::VulkanRenderer;
	};

}