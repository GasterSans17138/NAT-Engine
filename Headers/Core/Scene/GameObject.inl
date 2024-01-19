
namespace Core::Scene
{
	template<class T> inline T* GameObject::AddComponent()
	{
//#ifndef NDEBUG
//		T* out = new T();
//		Components::IComponent* res = dynamic_cast<Components::IComponent*>(out);
//		Assert(res != nullptr);
//		delete out;
//#endif
		T* result = new T();

		if (result == nullptr)
		{
			//Add better logging lol
			LOG(DEBUG_LEVEL::LERROR, "Error while creating component");
			return nullptr;
		}
		result->gameObject = this;
		components.push_back(result);
		result->Init();
		return result;
	}
}