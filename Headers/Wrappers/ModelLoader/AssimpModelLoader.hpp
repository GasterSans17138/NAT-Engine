#pragma once

#include "assimp/scene.h"

#include "Resources/Model.hpp"

#include "Wrappers/ModelLoader/IModelLoader.hpp"
#include "Renderer/VulkanRenderer.hpp"

namespace Wrappers::ModelLoader
{
	class NAT_API AssimpModelLoader : public IModelLoader
	{
	public:
		static void ParseModel(const char* pModelFilePath, Resources::Model* pModelOutput);
	private :
		static void ProcessAssimpScene(const aiScene* pScene, Resources::Model* pModelOutput, const std::string& path);
		static void ReadMaterialProperties(Resources::Material*& modelMat, aiMaterial* matIn, const std::string& meshName, const std::string& path);
		static void ReadMeshVertices(Resources::Mesh*& modelMesh, const aiMesh* meshIn);
		static void ReadTexture(Resources::StaticTexture*& tex, const std::string& folder, const char* path, bool isNormal = false);
		static Resources::ResourceManager* resources;
		static Renderer::VulkanRenderer* renderer;
	};
}