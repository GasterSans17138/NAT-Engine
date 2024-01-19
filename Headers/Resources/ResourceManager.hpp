#pragma once

#include <unordered_map>
#include <filesystem>

#include "Core/Types.hpp"
#include "Core/PRNG.hpp"
#include "Resources/IResource.hpp"

namespace Core
{
	class App;

	namespace Scene
	{
		class Scene;
	}
}

namespace LowRenderer
{
	class FrameBuffer;
	class DepthBuffer;
}

namespace Resources
{
	enum class NAT_API ObjectType : u8
	{
		ModelType = 0,
		SkinnedModelType,
		TextureSamplerType,
		TextureType,
		MaterialType,
		MeshType,
		VertexShaderType,
		FragmentShaderType,
		GeometryShaderType,
		ShaderProgramType,
		FrameBufferType,
		DepthBufferType,
		CubeMapType,
		SoundType,
		SoundDataType,
		SceneType,
		Size
	};
	class Texture;
	class StaticTexture;
	class TextureSampler;
	class Material;
	class Mesh;
	class Model;
	class ShaderProgram;
	class FragmentShader;
	class VertexShader;
	class GeometryShader;
	class CubeMap;
	class StaticCubeMap;
	class Sound;
	class SoundData;

	class NAT_API ResourceManager
	{
	public:

		ResourceManager() = default;
		ResourceManager(const ResourceManager&) = delete;
		ResourceManager(ResourceManager&) = delete;
		ResourceManager(ResourceManager&&) = default;

		~ResourceManager() = default;

		ResourceManager& operator=(ResourceManager&&) = default;

		IResource* Load(u64 hash, bool isHidden = false);
		template<typename T> T* Get(u64 hash);
		void Release(Resources::IResource* res);
		bool IsLoaded(u64 hash);
		void SaveAsset(IResource* resource);
		void SaveAllAssets();
		void Unload(IResource* resource);
		template<typename T> T* CreateResource(const std::string& path);
		template<typename T> T* CreateFixedResource(const std::string& path, u64 hash, bool isPublic = true);

		void DeleteAllResources();
		void DeleteResource(IResource* res);
		void EnableAutoDelete();

		u64 GetUniqueRandomHash();

		std::vector<Texture*>& GetTextureList();
		std::vector<Material*>& GetMaterialList();
		std::vector<Mesh*>& GetMeshList();
		std::vector<Model*>& GetModelList();
		std::vector<ShaderProgram*>& GetShaderProgramList();
		std::vector<FragmentShader*>& GetFragmentShaderList();
		std::vector<VertexShader*>& GetVertexShaderList();
		std::vector<GeometryShader*>& GetGeometryShaderList();
		std::vector<TextureSampler*>& GetTextureSamplerList();
		std::vector<CubeMap*>& GetCubeMapList();
		std::vector<Sound*>& GetSoundList();
		std::vector<SoundData*>& GetSoundDataList();
		std::vector<Core::Scene::Scene*>& GetSceneList();


	private:
		template<typename T> void PushResource(T* res);

		std::unordered_map<u64, std::unique_ptr<IResource>> resourceList;
		std::vector<Texture*> textureCached;
		std::vector<CubeMap*> cubeMapCached;
		std::vector<TextureSampler*> samplerCached;
		std::vector<Material*> materialCached;
		std::vector<Mesh*> meshCached;
		std::vector<Model*> modelCached;
		std::vector<ShaderProgram*> shaderProgramCached;
		std::vector<FragmentShader*> fragmentShaderCached;
		std::vector<VertexShader*> vertexShaderCached;
		std::vector<GeometryShader*> geometryShaderCached;
		std::vector<Sound*> soundCached;
		std::vector<SoundData*> soundDataCached;
		std::vector<Core::Scene::Scene*> sceneCached;
		Core::PRNG rng;
		bool autoDelete = false;
		friend Core::App;
	};

}

#include "ResourceManager.inl"