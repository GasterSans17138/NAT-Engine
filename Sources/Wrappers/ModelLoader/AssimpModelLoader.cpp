#include <vector>
#include <filesystem>

#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"
#include "assimp/cimport.h"

#include "Maths/Maths.hpp"

#include "Core/Debugging/Log.hpp"
#include "Renderer/RendererVertex.hpp"
#include "Core/App.hpp"
#include "Wrappers/ModelLoader/AssimpModelLoader.hpp"
#include "Wrappers/ImageLoader.hpp"

#include "Resources/ResourceManager.hpp"

#ifdef _WIN32
#pragma warning(disable : 26812)
#endif

using namespace Wrappers::ModelLoader;

Resources::ResourceManager* AssimpModelLoader::resources = nullptr;
Renderer::VulkanRenderer* AssimpModelLoader::renderer = nullptr;

void AssimpModelLoader::ParseModel(const char* pModelFilePath, Resources::Model* pModelOutput)
{
	if (!resources)
	{
		resources = &Core::App::GetInstance()->GetResources();
		renderer = &Core::App::GetInstance()->GetRenderer();
	}
	Assimp::Importer importer;

	/*
	* 
	* If you want to tune the importer feel free to change the flags.
	* Documentation :
	*	https://assimp.sourceforge.net/lib_html/postprocess_8h.html#a64795260b95f5a4b3f3dc1be4f52e410
	* 	
	*/

	const aiScene* scene = importer.ReadFile(pModelFilePath,
		aiProcess_Triangulate | 
		aiProcess_ValidateDataStructure | 
		aiProcess_RemoveRedundantMaterials |
		//aiProcess_FixInfacingNormals |
		aiProcess_FindInvalidData | 
		aiProcess_FindInstances |
		aiProcess_OptimizeMeshes | 
		aiProcess_JoinIdenticalVertices |
		aiProcess_CalcTangentSpace |
		aiProcess_GenBoundingBoxes |
		aiProcess_FlipUVs
	);

	if (scene == nullptr)
	{
		LOG(DEBUG_LEVEL::LERROR, importer.GetErrorString());
		return;
	}
	std::string path = std::filesystem::path(pModelFilePath).parent_path().string();
	AssimpModelLoader::ProcessAssimpScene(scene, pModelOutput, path);

	return;
}

void AssimpModelLoader::ProcessAssimpScene(const aiScene* pScene, Resources::Model* pModelOutput, const std::string& path)
{
	if (!pScene->HasMeshes())
		return;
	
	std::vector<Resources::Mesh*>& meshes = pModelOutput->meshes;
	std::vector<Resources::Material*>& materials = pModelOutput->materials;
	meshes.resize(pScene->mNumMeshes);
	materials.resize(pScene->mNumMeshes);

	pModelOutput->shader = Resources::ShaderProgram::GetDefaultShader();

	LOG_DEFAULT("Model Loader : Number of meshes : %i", pScene->mNumMeshes);

	for (u32 i = 0; i < pScene->mNumMeshes; i++)
	{
		const aiMesh* currentMesh = pScene->mMeshes[i];

		LOG_DEFAULT("Model Loader : Mesh : Uv channels of the Mesh : %i", currentMesh->GetNumUVChannels());
		LOG_DEFAULT("Model Loader : Mesh : Vertices of the Mesh : %i", currentMesh->mNumVertices);
		LOG_DEFAULT("Model Loader : Mesh : Faces of the Mesh : %i", currentMesh->mNumFaces);
		//LOG_DEFAULT("Model Loader : Mesh : Indices of the Mesh : %i", currentMesh->mNumFaces);

		aiMaterial* mat = pScene->mMaterials[currentMesh->mMaterialIndex];

		std::string meshName;
		// trying to read mesh name, defaults to "model name + mesh index" if none
		if (currentMesh->mName.C_Str())
		{
			meshName = currentMesh->mName.C_Str();
		}
		else
		{
			meshName = pScene->mName.C_Str() + Maths::Util::GetHex(i);
		}

		meshes[i] = resources->CreateResource<Resources::Mesh>(meshName);
		meshes[i]->aabb.size = Maths::Vec3(currentMesh->mAABB.mMax - currentMesh->mAABB.mMin);
		meshes[i]->aabb.center = meshes[i]->aabb.size / 2 + currentMesh->mAABB.mMin;
		ReadMaterialProperties(materials[i], mat, meshName, path);

		ReadMeshVertices(meshes[i], currentMesh);
	}

}

void AssimpModelLoader::ReadTexture(Resources::StaticTexture*& texture, const std::string& folder, const char* path, bool isNormal)
{
	std::filesystem::path p = path;
	if (!std::filesystem::exists(p))
	{
		p = folder + "/" + path;
		if (!std::filesystem::exists(p))
		{
			texture = isNormal ? Resources::StaticTexture::GetDefaultNormal() : Resources::StaticTexture::GetDefaultTexture();
		}
	}
	texture = resources->CreateResource<Resources::StaticTexture>(p.filename().string());
	if (Wrappers::ImageLoader::ParseTexture(p.string().c_str(), texture))
	{
		texture->isLoaded = renderer->LoadTexture(texture);
	}
	else
	{
		texture = isNormal ? Resources::StaticTexture::GetDefaultNormal() : Resources::StaticTexture::GetDefaultTexture();
	}
}

void AssimpModelLoader::ReadMaterialProperties(Resources::Material*& modelMat, aiMaterial* matIn, const std::string& meshName, const std::string& path)
{
	std::string matName;
	aiString name;
	if (matIn->Get(AI_MATKEY_NAME, name))
	{
		LOG(DEBUG_LEVEL::LWARNING, "Could not retrieve material name for mesh %s!", meshName.c_str());
		matName = meshName;
	}
	else
	{
		matName = name.C_Str();
	}
	modelMat = resources->CreateResource<Resources::Material>(matName);

	aiColor3D color(1.f, 1.f, 1.f);
	if (matIn->Get(AI_MATKEY_COLOR_DIFFUSE, color))
	{
		LOG(DEBUG_LEVEL::LWARNING, "Could not retrieve diffuse color for material %s!", matName.c_str());
	}
	modelMat->ambientColor = Maths::Vec3(color.r, color.g, color.b);
	/*
	if (matIn->Get(AI_MATKEY_COLOR_AMBIENT, color))
	{
		LOG(DEBUG_LEVEL::LWARNING, "Could not retrieve ambient color for material %s!", matName.c_str());
	}
	modelMat->ambientColor = Maths::Vec3(color.r, color.g, color.b);
	if (matIn->Get(AI_MATKEY_COLOR_SPECULAR, color))
	{
		LOG(DEBUG_LEVEL::LWARNING, "Could not retrieve specular color for material %s!", matName.c_str());
	}
	modelMat->specularColor = Maths::Vec3(color.r, color.g, color.b);
	*/
	ai_real shininess = 256;
	if (matIn->Get(AI_MATKEY_SHININESS, shininess))
	{
		LOG(DEBUG_LEVEL::LWARNING, "Could not retrieve shininess for material %s!", matName.c_str());
	}
	modelMat->shininess = Maths::Util::MaxF(shininess, 1.0f);

	aiString tex = {};
	if (matIn->GetTextureCount(aiTextureType_DIFFUSE))
	{
		matIn->Get(AI_MATKEY_TEXTURE_DIFFUSE(0), tex);
		if (tex.C_Str() && tex.C_Str()[0])
		{
			ReadTexture(reinterpret_cast<Resources::StaticTexture*&>(modelMat->albedo), path, tex.C_Str());
		}
		else
		{
			LOG(DEBUG_LEVEL::LWARNING, "Could not retrieve albedo texture for material %s!", matName.c_str());
			LOG(DEBUG_LEVEL::LWARNING, aiGetErrorString());
		}
	}

	tex.Clear();
	if (matIn->GetTextureCount(aiTextureType_NORMALS))
	{
		matIn->Get(AI_MATKEY_TEXTURE_NORMALS(0), tex);
		if (tex.C_Str() && tex.C_Str()[0])
		{
			ReadTexture(reinterpret_cast<Resources::StaticTexture*&>(modelMat->normal), path, tex.C_Str(), true);
		}
		else
		{
			LOG(DEBUG_LEVEL::LWARNING, "Could not retrieve normal map texture for material %s!", matName.c_str());
			LOG(DEBUG_LEVEL::LWARNING, aiGetErrorString());
		}
	}

	tex.Clear();
	if (matIn->GetTextureCount(aiTextureType_HEIGHT))
	{
		matIn->Get(AI_MATKEY_TEXTURE_HEIGHT(0), tex);
		if (tex.C_Str() && tex.C_Str()[0])
		{
			ReadTexture(reinterpret_cast<Resources::StaticTexture*&>(modelMat->height), path, tex.C_Str());
		}
		else
		{
			LOG(DEBUG_LEVEL::LWARNING, "Could not retrieve height map texture for material %s!", matName.c_str());
		}
	}
}

void AssimpModelLoader::ReadMeshVertices(Resources::Mesh*& modelMesh, const aiMesh* meshIn)
{
	std::vector<Renderer::RendererVertex> vertices(meshIn->mNumVertices);

	//
	//vertex parsing
	//

	for (u32 j = 0; j < meshIn->mNumVertices; j++)
	{
		Renderer::RendererVertex* currentVertex = &vertices[j];

		if (meshIn->HasPositions())
		{
			currentVertex->pos = meshIn->mVertices[j];
		}

		if (meshIn->HasNormals())
		{
			currentVertex->normal = meshIn->mNormals[j];
		}

		if (meshIn->HasTangentsAndBitangents())
		{
			currentVertex->tangent = meshIn->mTangents[j];
		}

		//
		//In the future we might want to work with more than one uv channel, assimp support it.
		//

		constexpr s32 uvChannel = 0;

		if (meshIn->HasTextureCoords(uvChannel))
		{
			currentVertex->uv.x = meshIn->mTextureCoords[uvChannel][j].x;
			currentVertex->uv.y = meshIn->mTextureCoords[uvChannel][j].y;
		}

		//
		//Same as uv's
		//

		constexpr s32 colorChannel = 0;

		if (meshIn->HasVertexColors(colorChannel))
		{
			currentVertex->color.x = meshIn->mColors[colorChannel][j].r;
			currentVertex->color.y = meshIn->mColors[colorChannel][j].g;
			currentVertex->color.z = meshIn->mColors[colorChannel][j].b;
			//currentVertex->color.w = currentMesh->mColors[uvIndex]->a;
		}
	}

	//
	//Indices parsing
	//

	std::vector<u32> indices(meshIn->mNumFaces * 3llu);

	for (u32 j = 0; j < meshIn->mNumFaces; j++)
	{
		for (u32 k = 0; k < meshIn->mFaces[j].mNumIndices; k++)
		{
			indices[(j * 3llu) + k] = meshIn->mFaces[j].mIndices[k];
		}
	}

	modelMesh->vertices = std::move(vertices);
	modelMesh->indices = std::move(indices); 
	modelMesh->UpdateVectors();
}
