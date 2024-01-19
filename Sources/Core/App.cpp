#include "Core/App.hpp"

#include <thread>

#include "Core/Debugging/Log.hpp"

#include "Wrappers/ImageLoader.hpp"
#include "Wrappers/ShaderLoader.hpp"
#include "Wrappers/ModelLoader/AssimpModelLoader.hpp"

#include "Resources/Shader.hpp"
#include "Resources/ShaderProgram.hpp"
#include "Resources/Model.hpp"

#include "Core/FileManager.hpp"

#include "Core/Scene/Components/Colliders/CubeCollider.hpp"
#include "Core/Scene/Components/IPhysicalComponent.hpp"

#include "Core/Serialization/Conversion.hpp"

#include "Core/Scene/Components/Lights/DirectionalLightComponent.hpp"
#include "Core/Scene/Components/SkyBoxComponent.hpp"

using namespace Core;

const char* requiredFolders[] =
{
	"Cache",
	"Cache/Shaders",
	"Default_Resources", // if you are missing this one, I can't help you...
	"Logs", // Already checked by Log class, adding it for concistency
};

App::App() : soundEngine(), soundLoader(soundEngine.GetEngine())
{
	appInstance = this;
}

 App* App::GetInstance()
{
	 return appInstance;
}

void Core::App::TakeScreenshot()
{
	LowRenderer::FrameBuffer fb = renderer.GetMainLightBuffer();
	Wrappers::ImageLoader::WriteStbi("output.hdr", fb.resolution.x, fb.resolution.y, 4, reinterpret_cast<const f32*>(renderer.ReadTexture(&fb, Maths::IVec2(), fb.resolution, sizeof(f32) * 4).data()), false);
}

Renderer::VulkanRenderer& App::GetRenderer()
{
	return renderer;
}

Wrappers::Interfacing& App::GetInterfacing()
{
	return guiInterface;
}

Resources::ResourceManager& App::GetResources()
{
	return resourceManager;
}

Wrappers::WindowManager& App::GetWindow()
{
	return window;
}

Wrappers::Sound::SoundEngine& Core::App::GetSoundEngine()
{
	return soundEngine;
}

Wrappers::Sound::SoundLoader& Core::App::GetSoundLoader()
{
	return soundLoader;
}

Scene::SceneManager& App::GetSceneManager()
{
	return sceneManager;
}

Wrappers::PhysicsEngine::JoltPhysicsEngine& Core::App::GetPhysicsEngine()
{
	return physicsEngine;
}


void App::CreateFolders()
{
	for (auto& path : requiredFolders)
	{
		std::filesystem::path p = std::filesystem::current_path().append(path);
		if (!std::filesystem::exists(p))
		{
			std::filesystem::create_directory(p);
		}
	}
}

bool App::LoadProjectSettings()
{
	std::filesystem::path p = "ProjectSettings.bin";
	if (std::filesystem::exists(p))
	{
		auto content = FileManager::LoadFileAsVector(p.string().c_str());
		Serialization::Deserializer dr(content.data(), content.size());
		dr.Read(defaultResolution);
		bool fullScreen;
		dr.Read(fullScreen);
		dr.Read(defaultSceneHash);
		u64 size = 0;
		dr.Read(size);
		if (!size || size + dr.CursorPos() > dr.BufferSize()) return false;
		postProcesssData.resize(size);
		dr.Read(postProcesssData.data(), size);
		return fullScreen;
	}
	if (!defaultSceneHash) defaultSceneHash = 0x69;
	return false;
}

void Core::App::SaveProjectSettings()
{
	Serialization::Serializer sr;
	sr.Write(window.GetWindowSize());
	sr.Write(window.IsFullScreen());
	sr.Write(defaultScene->hash);
	Serialization::Serializer sr2;
	sceneManager.postManager.Serialize(sr2);
	sr.Write(sr2.GetBufferSize());
	sr.Write(sr2.GetBuffer(), sr2.GetBufferSize());
	FileManager::WriteFile("ProjectSettings.bin", sr.GetBuffer(), sr.GetBufferSize());
}

void App::LoadResourcesAlreadyCached()
{
	std::string path = requiredFolders[0];
	for (u64 i = 0x10; i < 0x70; i++)
	{
		u64 value;
		Serialization::Conversion::ToNetwork(i, value);
		std::string file = path + "/" + Maths::Util::GetHex(value) + ".ass";
		if (Core::FileManager::FileExist(file.c_str()))
		{
			Resources::IResource* res = resourceManager.Load(i);
			res->isSaved = true;
			switch (i)
			{
			case 0x10:
				reinterpret_cast<Resources::TextureSampler*>(res)->SetDefaultSampler();
				DefaultSamplerLoaded();
				break;
			case 0x11:
				reinterpret_cast<Resources::StaticTexture*>(res)->SetDebugTexture();
				break;
			case 0x12:
				reinterpret_cast<Resources::StaticTexture*>(res)->SetDefaultTexture();
				break;
			case 0x13:
				reinterpret_cast<Resources::StaticTexture*>(res)->SetDefaultNormal();
				renderer.InitRendererData();
				break;
			case 0x19:
				reinterpret_cast<Resources::ShaderProgram*>(res)->SetDefaultShader();
				break;
			case 0x1a:
				reinterpret_cast<Resources::Material*>(res)->SetDefaultMaterial();
				break;
			case 0x32:
				reinterpret_cast<Resources::StaticCubeMap*>(res)->SetDebugCubeMap();
				break;
			case 0x33:
				reinterpret_cast<Resources::StaticCubeMap*>(res)->SetDefaultCubeMap();
				break;
			default:
				break;
			}
		}
	}

	for (const auto& entry : std::filesystem::directory_iterator(path, std::filesystem::directory_options::skip_permission_denied))
	{
		u64 hashIn = Maths::Util::ReadHex(entry.path().filename().string());
		u64 hash;
		Serialization::Conversion::ToLocal(hashIn, hash);
		if (hash < 0x70) continue;
		resourceManager.Load(hash);
	}
}

void App::DefaultSamplerLoaded()
{

}

