#include "Core/Debugging/Log.hpp"

#include "Wrappers/ModelLoader/IModelLoader.hpp"

using namespace Wrappers::ModelLoader;

void IModelLoader::ParseModel(const char* pModelFilePath, Resources::Model* pModelOutpout)
{
	LOG(DEBUG_LEVEL::LERROR, "%s ; %s : Model Loader wrapper : Absctract was used to load a model, please use/implement a proper wrapper", __FILE__, __LINE__);
	return;
}