#pragma once

#include "IResource.hpp"
#include "Renderer/RendererShader.hpp"
#include "ResourceManager.hpp"

namespace Resources
{
	class ShaderProgram;
	class NAT_API Shader : public IResource
	{
	public:
		Shader() = default;

		void DeleteData() override;
		virtual Renderer::RendererShader& GetRendererShader() = 0;
		virtual void Write(Core::Serialization::Serializer& sr) override;
		void Load(Core::Serialization::Deserializer& dr) override;
		virtual void WindowCreateResource(bool& open) override = 0;
		std::string shaderData;
	};

	class NAT_API FragmentShader : public Shader
	{
	public:
		FragmentShader();
		~FragmentShader() override = default;

		void Write(Core::Serialization::Serializer& sr) override;
		void WindowCreateResource(bool& open) override;
		ObjectType GetType() override { return ObjectType::FragmentShaderType; }

		Renderer::RendererShader& GetRendererShader() override { return shader; }
	private:
		Renderer::FragmentRendererShader shader;
		friend ShaderProgram;
	};

	class NAT_API VertexShader : public Shader
	{
	public:
		VertexShader();
		~VertexShader() override = default;

		void Write(Core::Serialization::Serializer& sr) override;
		void WindowCreateResource(bool& open) override;
		ObjectType GetType() override { return ObjectType::VertexShaderType; }

		Renderer::RendererShader& GetRendererShader() override { return shader; }
	private:
		Renderer::VertexRendererShader shader;
		friend ShaderProgram;
	};

	class NAT_API GeometryShader : public Shader
	{
	public:
		GeometryShader();
		~GeometryShader() override = default;

		void Write(Core::Serialization::Serializer& sr) override;
		void WindowCreateResource(bool& open) override;
		ObjectType GetType() override { return ObjectType::GeometryShaderType; }

		Renderer::RendererShader& GetRendererShader() override { return shader; }
	private:
		Renderer::GeometryRendererShader shader;
		friend ShaderProgram;
	};

}