#include "Core/Scene/Components/Sounds/SoundListenerComponent.hpp"
#include "Wrappers/Sound/SoundEngine.hpp"
#include "Core/App.hpp"

namespace Core::Scene::Components::Sounds
{

	IComponent* SoundListenerComponent::CreateCopy()
	{
		return new SoundListenerComponent(*this);
	}

	ComponentType SoundListenerComponent::GetType()
	{
		return ComponentType::SoundListenerComponent;
	}

	void SoundListenerComponent::RenderGui()
	{

	}

	const char* SoundListenerComponent::GetName()
	{
		return "Sound Listener";
	}

	void SoundListenerComponent::Serialize(Core::Serialization::Serializer& sr) const
	{
		sr.Write(cachedPos);
	}

	void SoundListenerComponent::Deserialize(Core::Serialization::Deserializer& dr)
	{
		dr.Read(cachedPos);
	}

	void SoundListenerComponent::Update()
	{
		const Maths::Mat4& global = gameObject->transform.GetGlobal();
		Maths::Vec3 pos = global.GetPositionFromTranslation();
		App::GetInstance()->GetSoundEngine().UpdateListener(pos, (global * Maths::Vec4(0, 0, 1, 1)).GetVector() - pos, (global * Maths::Vec4(0, 1, 0, 1)).GetVector() - pos, (pos - cachedPos) / App::GetInstance()->GetWindow().GetDeltaTime());
		cachedPos = pos;
	}
}