#include "Resources/StaticCubeMap.hpp"

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

StaticCubeMap* StaticCubeMap::debugCubeMap = nullptr;
StaticCubeMap* StaticCubeMap::defaultCubeMap = nullptr;

StaticCubeMap::StaticCubeMap()
{
}

void StaticCubeMap::DeleteData()
{
	DeleteCubeMapData();
	app->GetRenderer().UnLoadCubeMap(this);
}

void StaticCubeMap::DeleteCubeMapData()
{
	if (cubeMapData != nullptr)
	{
		Wrappers::ImageLoader::FreeStbi(cubeMapData);
		cubeMapData = nullptr;
	}
}

void StaticCubeMap::Write(Serializer& sr)
{
	sr.Write(static_cast<u8>(ObjectType::CubeMapType));
	CubeMap::Write(sr);
	sr.Write(isFloat);
	if (isFloat)
	{
		for (u64 i = 0; i < 6 * 4llu * resolution.x * resolution.y; i++)
		{
			sr.Write(reinterpret_cast<f32*>(cubeMapData)[i]);
		}
	}
	else
	{
		sr.Write(reinterpret_cast<u8*>(cubeMapData), 6 * 4llu * resolution.x * resolution.y);
	}
}

void StaticCubeMap::Load(Deserializer& dr)
{
	CubeMap::Load(dr);
	dr.Read(isFloat);
	if (isFloat)
	{
		cubeMapData = malloc(sizeof(f32) * 6 * 4llu * resolution.x * resolution.y);
		for (u64 i = 0; i < 6 * 4llu * resolution.x * resolution.y; i++)
		{
			dr.Read(reinterpret_cast<f32*>(cubeMapData)[i]);
		}
	}
	else
	{
		cubeMapData = malloc(6 * 4llu * resolution.x * resolution.y);
		dr.Read(reinterpret_cast<u8*>(cubeMapData), 6 * 4llu * resolution.x * resolution.y);
	}
	UpdateMipLevel();
	isLoaded = app->GetRenderer().LoadCubeMap(this);
}

void Resources::StaticCubeMap::WindowCreateResource(bool& open)
{
	
}

const Renderer::RendererImageView& StaticCubeMap::GetImageView() const
{
	return isLoaded ? cubeImageView.imageView : debugCubeMap->GetImageView();
}

const Renderer::RendererTexture& Resources::StaticCubeMap::GetRendererTexture() const
{
	return isLoaded ? cubeImageView : debugCubeMap->GetRendererTexture();
}

const Renderer::RendererTexture& Resources::StaticCubeMap::GetImGuiRendererTexture() const
{
	return isLoaded ? renderTexture : debugCubeMap->GetImGuiRendererTexture();
}

StaticCubeMap* Resources::StaticCubeMap::GetDebugCubeMap()
{
	return debugCubeMap;
}

StaticCubeMap* StaticCubeMap::GetDefaultCubeMap()
{
	return defaultCubeMap;
}

void Resources::StaticCubeMap::SetDebugCubeMap()
{
	debugCubeMap = this;
}

void StaticCubeMap::SetDefaultCubeMap()
{
	defaultCubeMap = this;
}
