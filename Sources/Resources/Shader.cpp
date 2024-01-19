#include "Resources/Shader.hpp"

#include "Renderer/VulkanRenderer.hpp"
#include "Core/App.hpp"
using namespace Resources;

FragmentShader::FragmentShader()
{
}

void FragmentShader::Write(Core::Serialization::Serializer& sr)
{
	sr.Write(static_cast<u8>(ObjectType::FragmentShaderType));
	Shader::Write(sr);
}

void FragmentShader::WindowCreateResource(bool& open)
{
	
}

VertexShader::VertexShader()
{
}

void VertexShader::Write(Core::Serialization::Serializer& sr)
{
	sr.Write(static_cast<u8>(ObjectType::VertexShaderType));
	Shader::Write(sr);
}

void VertexShader::WindowCreateResource(bool& open)
{
	
}

GeometryShader::GeometryShader()
{
}

void GeometryShader::Write(Core::Serialization::Serializer& sr)
{
	sr.Write(static_cast<u8>(ObjectType::GeometryShaderType));
	Shader::Write(sr);
}

void GeometryShader::WindowCreateResource(bool& open)
{
	
}

void Shader::DeleteData()
{
	app->GetRenderer().UnLoadShader(this);
}

void Shader::Write(Core::Serialization::Serializer& sr)
{
	IResource::Write(sr);
	sr.Write(shaderData);
}

void Shader::Load(Core::Serialization::Deserializer& dr)
{
	IResource::Load(dr);
	dr.Read(shaderData);
	isLoaded = app->GetRenderer().LoadShader(this);
}
	
