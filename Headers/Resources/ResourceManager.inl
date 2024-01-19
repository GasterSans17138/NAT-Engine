
#include "Core/Debugging/Assert.hpp"
#include "Core/Debugging/Log.hpp"

#include "ResourceManager.hpp"

template<typename T>
T* Resources::ResourceManager::Get(u64 hash)
{
    Assert(hash);
#ifndef NDEBUG
    T* out = new T();
    IResource* res = dynamic_cast<IResource*>(out);
    Assert(res != nullptr);
    delete out;
#endif

    auto result = resourceList.find(hash);
    if (result == resourceList.end())
    {
        LOG(DEBUG_LEVEL::LINFO, "Ressource %llu is not loaded, trying to load it from cache", hash);
        T* cachedResource = reinterpret_cast<T*>(Load(hash));

        if (cachedResource == nullptr)
        {
            LOG(DEBUG_LEVEL::LWARNING, "Failed to get Resource %lu, please check files integrity", hash);
        }
        else
        {
            LOG(DEBUG_LEVEL::LINFO, "Loaded ressource %llu from cache", hash);
            //PushResource<T>(cachedResource);
            cachedResource->refCount++;
        }
        return cachedResource;
    }
    result->second->refCount++;
    return reinterpret_cast<T*>(result->second.get());
}

template<typename T>
inline T* Resources::ResourceManager::CreateResource(const std::string& path)
{
    return CreateFixedResource<T>(path, GetUniqueRandomHash());
}

template<typename T>
inline T* Resources::ResourceManager::CreateFixedResource(const std::string& path, u64 hash, bool isPublic)
{
#ifndef NDEBUG
    T* out = new T();
    IResource* res = dynamic_cast<IResource*>(out);
    Assert(res != nullptr);
    delete out;
#endif
    std::unique_ptr<IResource> resource = std::make_unique<T>();
    resource->path = path;
    resource->hash = hash;
    T* ptr = reinterpret_cast<T*>(resource.get());
    if (isPublic) PushResource<T>(ptr);
    resourceList.emplace(hash, std::move(resource));
    return ptr;
}

template<>
inline void Resources::ResourceManager::PushResource<Resources::StaticTexture>(Resources::StaticTexture* res)
{
    textureCached.push_back(reinterpret_cast<Resources::Texture*>(res));
}

template<>
inline void Resources::ResourceManager::PushResource<Resources::StaticCubeMap>(Resources::StaticCubeMap* res)
{
    cubeMapCached.push_back(reinterpret_cast<Resources::CubeMap*>(res));
}

template<>
inline void Resources::ResourceManager::PushResource<LowRenderer::FrameBuffer>(LowRenderer::FrameBuffer* res)
{
    textureCached.push_back(reinterpret_cast<Resources::Texture*>(res));
}

template<>
inline void Resources::ResourceManager::PushResource<LowRenderer::DepthBuffer>(LowRenderer::DepthBuffer* res)
{
    textureCached.push_back(reinterpret_cast<Resources::Texture*>(res));
}

template<>
inline void Resources::ResourceManager::PushResource<Resources::ShaderProgram>(Resources::ShaderProgram* res)
{
    shaderProgramCached.push_back(res);
}

template<>
inline void Resources::ResourceManager::PushResource<Resources::FragmentShader>(Resources::FragmentShader* res)
{
    fragmentShaderCached.push_back(res);
}

template<>
inline void Resources::ResourceManager::PushResource<Resources::VertexShader>(Resources::VertexShader* res)
{
    vertexShaderCached.push_back(res);
}

template<>
inline void Resources::ResourceManager::PushResource<Resources::GeometryShader>(Resources::GeometryShader* res)
{
    geometryShaderCached.push_back(res);
}

template<>
inline void Resources::ResourceManager::PushResource<Resources::TextureSampler>(Resources::TextureSampler* res)
{
    samplerCached.push_back(res);
}

template<>
inline void Resources::ResourceManager::PushResource<Resources::Material>(Resources::Material* res)
{
    materialCached.push_back(res);
}

template<>
inline void Resources::ResourceManager::PushResource<Resources::Mesh>(Resources::Mesh* res)
{
    meshCached.push_back(res);
}

template<>
inline void Resources::ResourceManager::PushResource<Resources::Model>(Resources::Model* res)
{
    modelCached.push_back(res);
}

template<>
inline void Resources::ResourceManager::PushResource<Resources::Sound>(Resources::Sound* res)
{
    soundCached.push_back(res);
}

template<>
inline void Resources::ResourceManager::PushResource<Resources::SoundData>(Resources::SoundData* res)
{
    soundDataCached.push_back(res);
}

template<>
inline void Resources::ResourceManager::PushResource<Core::Scene::Scene>(Core::Scene::Scene* res)
{
    sceneCached.push_back(res);
}

template<typename T>
inline void Resources::ResourceManager::PushResource(T* res)
{
    LOG(DEBUG_LEVEL::LWARNING, "Unsorted resource %lu : &s", res->hash, res->path.c_str());
}
