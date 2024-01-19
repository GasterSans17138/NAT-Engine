#include "Resources/ResourceManager.hpp"

#include "Core/FileManager.hpp"
#include "Core/Serialization/Serializer.hpp"
#include "Core/Serialization/Deserializer.hpp"
#include "Resources/Model.hpp"
#include "Resources/StaticTexture.hpp"
#include "Resources/StaticCubeMap.hpp"
#include "LowRenderer/FrameBuffer.hpp"
#include "LowRenderer/DepthBuffer.hpp"
#include "Resources/Sound.hpp"
#include "Resources/SoundData.hpp"
#include "Core/Scene/Scene.hpp"

using namespace Resources;
using namespace Core::FileManager;
using namespace Core::Serialization;
using namespace Core::Serialization::Conversion;

IResource* ResourceManager::Load(u64 hashIn, bool isHidden)
{
    Assert(hashIn);
    auto found = resourceList.find(hashIn);
    if (found != resourceList.end())
    {
        return found->second.get();
    }
    u64 hash;
    ToNetwork(hashIn, hash);
    std::filesystem::path p = std::filesystem::current_path().append("Cache").append(Maths::Util::GetHex(hash) + ".ass"); // HAHA FUNNY HE SAID ASS
    if (p.empty() || !std::filesystem::exists(p))
    {
        Assert("Resource not find in cache folder");
        // TODO Remplace Assert by default resource
        return nullptr;
    }
    std::string path = p.string();
    std::vector<u8> data = LoadFileAsVector(path.c_str());
    Deserializer dr(data);
    u8 tmpType;
    if (!dr.Read(tmpType) || tmpType >= static_cast<u8>(ObjectType::Size)) return nullptr;
    ObjectType type = static_cast<ObjectType>(tmpType);
    std::unique_ptr<IResource> res = nullptr;
    switch (type)
    {
    case ObjectType::ModelType:
        res = std::make_unique<Model>();
        if (!isHidden) modelCached.push_back(reinterpret_cast<Model*>(res.get()));
        break;
    case ObjectType::SkinnedModelType: // TODO
        //res = std::make_unique<Model>();
        //if (!isHidden) modelCached.push_back(reinterpret_cast<Model*>(res.get()));
        break;
    case ObjectType::TextureSamplerType:
        res = std::make_unique<TextureSampler>();
        if (!isHidden) samplerCached.push_back(reinterpret_cast<TextureSampler*>(res.get()));
        break;
    case ObjectType::TextureType:
        res = std::make_unique<StaticTexture>();
        if (!isHidden) textureCached.push_back(reinterpret_cast<Texture*>(res.get()));
        break;
    case ObjectType::MaterialType:
        res = std::make_unique<Material>();
        if (!isHidden) materialCached.push_back(reinterpret_cast<Material*>(res.get()));
        break;
    case ObjectType::MeshType:
        res = std::make_unique<Mesh>();
        if (!isHidden) meshCached.push_back(reinterpret_cast<Mesh*>(res.get()));
        break;
    case ObjectType::VertexShaderType:
        res = std::make_unique<VertexShader>();
        if (!isHidden) vertexShaderCached.push_back(reinterpret_cast<VertexShader*>(res.get()));
        break;
    case ObjectType::FragmentShaderType:
        res = std::make_unique<FragmentShader>();
        if (!isHidden) fragmentShaderCached.push_back(reinterpret_cast<FragmentShader*>(res.get()));
        break;
    case ObjectType::GeometryShaderType:
        res = std::make_unique<GeometryShader>();
        if (!isHidden) geometryShaderCached.push_back(reinterpret_cast<GeometryShader*>(res.get()));
        break;
    case ObjectType::ShaderProgramType:
        res = std::make_unique<ShaderProgram>();
        if (!isHidden) shaderProgramCached.push_back(reinterpret_cast<ShaderProgram*>(res.get()));
        break;
    case ObjectType::FrameBufferType:
        res = std::make_unique<LowRenderer::FrameBuffer>();
        if (!isHidden) textureCached.push_back(reinterpret_cast<Texture*>(res.get()));
        break;
    case ObjectType::DepthBufferType:
        res = std::make_unique<LowRenderer::DepthBuffer>();
        if (!isHidden) textureCached.push_back(reinterpret_cast<Texture*>(res.get()));
        break;
    case ObjectType::CubeMapType:
        res = std::make_unique<StaticCubeMap>();
        if (!isHidden) cubeMapCached.push_back(reinterpret_cast<CubeMap*>(res.get()));
        break;
    case ObjectType::SoundType:
        res = std::make_unique<Sound>();
        if (!isHidden) soundCached.push_back(reinterpret_cast<Sound*>(res.get()));
        break;
    case ObjectType::SoundDataType:
        res = std::make_unique<SoundData>();
        if (!isHidden) soundDataCached.push_back(reinterpret_cast<SoundData*>(res.get()));
        break;
    case ObjectType::SceneType:
        res = std::make_unique<Core::Scene::Scene>();
        if (!isHidden) sceneCached.push_back(reinterpret_cast<Core::Scene::Scene*>(res.get()));
        break;
    default:
        LOG(DEBUG_LEVEL::LERROR, "Unimplemented asset type %d : are you a time traveller ?", static_cast<u32>(type));
        break;
    }
    res->hash = hashIn;
    res->Load(dr);
    IResource* result = res.get();
    result->isSaved = true;
    resourceList.emplace(hashIn, std::move(res));
    return result;
}

void ResourceManager::Release(Resources::IResource* res)
{
    if (!res) return;
    if (res->refCount) res->refCount--;
    if (!autoDelete || res->hash < 0x70 || res->refCount) return;
    Unload(res);
}

bool ResourceManager::IsLoaded(u64 hash)
{
    auto found = resourceList.find(hash);
    return found != resourceList.end();
}

void ResourceManager::SaveAsset(IResource* resource)
{
    resource->isSaved = true;
    u64 hash;
    ToNetwork(resource->hash, hash);
    std::filesystem::path p = std::filesystem::current_path().append("Cache").append(Maths::Util::GetHex(hash) + ".ass");
    std::string p_str = p.string();
    const char* path = p_str.c_str();
    Serializer sr;
    resource->Write(sr);
    WriteFile(path, sr.GetBuffer(), sr.GetBufferSize());
}

void ResourceManager::SaveAllAssets()
{
    for (auto& res : resourceList)
    {
        SaveAsset(res.second.get());
    }
}

void ResourceManager::Unload(IResource* resource)
{
    auto result = resourceList.find(resource->hash);
    if (result == resourceList.end()) return;
    result->second->DeleteData();
    resourceList.erase(result);
}

void ResourceManager::DeleteAllResources()
{
    fragmentShaderCached.clear();
    vertexShaderCached.clear();
    geometryShaderCached.clear();
    meshCached.clear();
    modelCached.clear();
    textureCached.clear();
    samplerCached.clear();
    shaderProgramCached.clear();
    for (auto& res : resourceList)
    {
        res.second->DeleteData();
    }
    resourceList.clear();
}

void ResourceManager::DeleteResource(IResource* res)
{
    Assert(res);
    u64 hash;
    ToNetwork(res->hash, hash);
    std::filesystem::path p = std::filesystem::current_path().append("Cache").append(Maths::Util::GetHex(hash) + ".ass");
    if (std::filesystem::exists(p))
    {
        std::filesystem::remove(p);
    }
    switch (res->GetType())
    {
    case ObjectType::ModelType:
        
        for (u64 i = 0; i < modelCached.size(); i++)
        {
            if (res == modelCached[i])
            {
                for (u64 j = i; j < modelCached.size() - 1; j++)
                {
                    modelCached[j] = modelCached[j + 1];
                }
                modelCached.pop_back();
            }
        }
        break;
    case ObjectType::SkinnedModelType:
        break;
    case ObjectType::TextureSamplerType:
        for (u64 i = 0; i < samplerCached.size(); i++)
        {
            if (res == samplerCached[i])
            {
                for (u64 j = i; j < samplerCached.size() - 1; j++)
                {
                    samplerCached[j] = samplerCached[j + 1];
                }
                samplerCached.pop_back();
            }
        }
        break;
    case ObjectType::TextureType:
        for (u64 i = 0; i < textureCached.size(); i++)
        {
            if (res == textureCached[i])
            {
                for (u64 j = i; j < textureCached.size() - 1; j++)
                {
                    textureCached[j] = textureCached[j + 1];
                }
                textureCached.pop_back();
            }
        }
        break;
    case ObjectType::MaterialType:
        for (u64 i = 0; i < materialCached.size(); i++)
        {
            if (res == materialCached[i])
            {
                for (u64 j = i; j < materialCached.size() - 1; j++)
                {
                    materialCached[j] = materialCached[j + 1];
                }
                materialCached.pop_back();
            }
        }
        break;
    case ObjectType::MeshType:
        for (u64 i = 0; i < meshCached.size(); i++)
        {
            if (res == meshCached[i])
            {
                for (u64 j = i; j < meshCached.size() - 1; j++)
                {
                    meshCached[j] = meshCached[j + 1];
                }
                meshCached.pop_back();
            }
        }
        break;
    case ObjectType::VertexShaderType:
        for (u64 i = 0; i < vertexShaderCached.size(); i++)
        {
            if (res == vertexShaderCached[i])
            {
                for (u64 j = i; j < vertexShaderCached.size() - 1; j++)
                {
                    vertexShaderCached[j] = vertexShaderCached[j + 1];
                }
                vertexShaderCached.pop_back();
            }
        }
        break;
    case ObjectType::FragmentShaderType:
        for (u64 i = 0; i < fragmentShaderCached.size(); i++)
        {
            if (res == fragmentShaderCached[i])
            {
                for (u64 j = i; j < fragmentShaderCached.size() - 1; j++)
                {
                    fragmentShaderCached[j] = fragmentShaderCached[j + 1];
                }
                fragmentShaderCached.pop_back();
            }
        }
        break;
    case ObjectType::GeometryShaderType:
        for (u64 i = 0; i < geometryShaderCached.size(); i++)
        {
            if (res == geometryShaderCached[i])
            {
                for (u64 j = i; j < geometryShaderCached.size() - 1; j++)
                {
                    geometryShaderCached[j] = geometryShaderCached[j + 1];
                }
                geometryShaderCached.pop_back();
            }
        }
        break;
    case ObjectType::ShaderProgramType:
        for (u64 i = 0; i < shaderProgramCached.size(); i++)
        {
            if (res == shaderProgramCached[i])
            {
                for (u64 j = i; j < shaderProgramCached.size() - 1; j++)
                {
                    shaderProgramCached[j] = shaderProgramCached[j + 1];
                }
                shaderProgramCached.pop_back();
            }
        }
        break;
    case ObjectType::SoundType:
        for (u64 i = 0; i < soundCached.size(); i++)
        {
            if (res == soundCached[i])
            {
                for (u64 j = i; j < soundCached.size() - 1; j++)
                {
                    soundCached[j] = soundCached[j + 1];
                }
                soundCached.pop_back();
            }
        }
        break;
    case ObjectType::SoundDataType:
        for (u64 i = 0; i < soundDataCached.size(); i++)
        {
            if (res == soundDataCached[i])
            {
                for (u64 j = i; j < soundDataCached.size() - 1; j++)
                {
                    soundDataCached[j] = soundDataCached[j + 1];
                }
                soundDataCached.pop_back();
            }
        }
        break;
    case ObjectType::SceneType:
        for (u64 i = 0; i < sceneCached.size(); i++)
        {
            if (res == sceneCached[i])
            {
                for (u64 j = i; j < sceneCached.size() - 1; j++)
                {
                    sceneCached[j] = sceneCached[j + 1];
                }
                sceneCached.pop_back();
            }
        }
        break;
    default:
        LOG(DEBUG_LEVEL::LWARNING, "Trying to delete unknown resource %s of type %d", res->path.c_str(), res->GetType());
        break;
    }
}

void ResourceManager::EnableAutoDelete()
{
    autoDelete = true;
}

u64 ResourceManager::GetUniqueRandomHash()
{
    u64 hash;
    do
    {
        hash = rng.NextLong();
    }
    while (hash < 0x70 && resourceList.find(hash) != resourceList.end());
    return hash;
}

std::vector<Texture*>& ResourceManager::GetTextureList()
{
    return textureCached;
}
std::vector<Material*>& ResourceManager::GetMaterialList()
{
    return materialCached;
}
std::vector<Mesh*>& ResourceManager::GetMeshList()
{
    return meshCached;
}
std::vector<Model*>& ResourceManager::GetModelList()
{
    return modelCached;
}
std::vector<ShaderProgram*>& ResourceManager::GetShaderProgramList()
{
    return shaderProgramCached;
}
std::vector<FragmentShader*>& ResourceManager::GetFragmentShaderList()
{
    return fragmentShaderCached;
}
std::vector<VertexShader*>& ResourceManager::GetVertexShaderList()
{
    return vertexShaderCached;
}
std::vector<GeometryShader*>& ResourceManager::GetGeometryShaderList()
{
    return geometryShaderCached;
}

std::vector<TextureSampler*>& ResourceManager::GetTextureSamplerList()
{
    return samplerCached;
}

std::vector<CubeMap*>& ResourceManager::GetCubeMapList()
{
    return cubeMapCached;
}

std::vector<Sound*>& ResourceManager::GetSoundList()
{
    return soundCached;
}

std::vector<SoundData*>& ResourceManager::GetSoundDataList()
{
    return soundDataCached;
}

std::vector<Core::Scene::Scene*>& ResourceManager::GetSceneList()
{
    return sceneCached;
}
