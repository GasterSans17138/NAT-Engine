#pragma once

#include "Renderer/RendererMesh.hpp"

#include "Resources/Model.hpp"

namespace Wrappers::ModelLoader
{
	class NAT_API IModelLoader
	{
	public:
		static void ParseModel(const char* pModelFilePath, Resources::Model* pModelOutpout);
	};
};