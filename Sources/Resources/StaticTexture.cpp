#include "Resources/StaticTexture.hpp"

#include <filesystem>

#include "Wrappers/ImageLoader.hpp"
#include "Core/Serialization/Serializer.hpp"
#include "Core/FileManager.hpp"
#include "Resources/ResourceManager.hpp"
#include "Renderer/VulkanRenderer.hpp"
#include "Core/App.hpp"

using namespace Resources;
using namespace Core::Serialization;
using namespace Core::FileManager;

StaticTexture* StaticTexture::debugTexture = nullptr;
StaticTexture* StaticTexture::defaultTexture = nullptr;
StaticTexture* StaticTexture::defaultNormal = nullptr;

StaticTexture::StaticTexture()
{
}

void StaticTexture::DeleteData()
{
	DeleteTextureData();
	app->GetRenderer().UnLoadTexture(this);
}

void StaticTexture::DeleteTextureData()
{
	if (textureData != nullptr)
	{
		Wrappers::ImageLoader::FreeStbi(textureData);
		textureData = nullptr;
	}
}

void StaticTexture::Write(Serializer& sr)
{
	sr.Write(static_cast<u8>(ObjectType::TextureType));
	Texture::Write(sr);
	sr.Write(isFloat);
	if (isFloat)
	{
		for (u64 i = 0; i < 4llu * resolution.x * resolution.y; i++)
		{
			sr.Write(reinterpret_cast<f32*>(textureData)[i]);
		}
	}
	else
	{
		sr.Write(reinterpret_cast<u8*>(textureData), 4llu * resolution.x * resolution.y);
	}
}

void StaticTexture::Load(Deserializer& dr)
{
	Texture::Load(dr);
	dr.Read(isFloat);
	if (isFloat)
	{
		textureData = malloc(sizeof(f32) * 4llu * resolution.x * resolution.y);
		for (u64 i = 0; i < 4llu * resolution.x * resolution.y; i++)
		{
			dr.Read(reinterpret_cast<f32*>(textureData)[i]);
		}
	}
	else
	{
		textureData = malloc(4llu * resolution.x * resolution.y);
		dr.Read(reinterpret_cast<u8*>(textureData), 4llu * resolution.x * resolution.y);
	}
	UpdateMipLevel();
	isLoaded = app->GetRenderer().LoadTexture(this);
}

void Resources::StaticTexture::WindowCreateResource(bool& open)
{
	
}

const Renderer::RendererImageView& StaticTexture::GetImageView() const
{
	return isLoaded ? renderTexture.imageView : debugTexture->GetImageView();
}

const Renderer::RendererTexture& Resources::StaticTexture::GetRendererTexture() const
{
	return isLoaded ? renderTexture : debugTexture->GetRendererTexture();
}

StaticTexture* Resources::StaticTexture::GetDebugTexture()
{
	return debugTexture;
}

StaticTexture* StaticTexture::GetDefaultTexture()
{
	return defaultTexture;
}

StaticTexture* StaticTexture::GetDefaultNormal()
{
	return defaultNormal;
}

void Resources::StaticTexture::SetDebugTexture()
{
	debugTexture = this;
}

void StaticTexture::SetDefaultTexture()
{
	defaultTexture = this;
}

void StaticTexture::SetDefaultNormal()
{
	defaultNormal = this;
}
