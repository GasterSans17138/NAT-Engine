#include "Core/Scene/Components/Sounds/SoundPlayerComponent.hpp"
#include "Core/App.hpp"
#include "Wrappers/Sound/SoundEngine.hpp"

namespace Core::Scene::Components::Sounds
{
	void SoundPlayerComponent::Init()
	{

	}

	IComponent* SoundPlayerComponent::CreateCopy()
	{
		return new SoundPlayerComponent(*this);
	}

	ComponentType SoundPlayerComponent::GetType()
	{
		return ComponentType::SoundPlayerComponent;
	}

	void SoundPlayerComponent::RenderGui()
	{
		interfaceGui->SoundListCombo(&sound);
		interfaceGui->Separator();
	}

	const char* SoundPlayerComponent::GetName()
	{
		return "Sound Player";
	}

	void SoundPlayerComponent::Update()
	{
		if (sound)
		{
			auto app = App::GetInstance();
			Maths::Vec3 vel = gameObject->transform.GetGlobal().GetPositionFromTranslation() - cachedPos;
			app->GetSoundEngine().SetVelocity(sound, vel / app->GetWindow().GetDeltaTime());
		}
	}

	void SoundPlayerComponent::DataUpdate()
	{
		if (sound)
		{
			cachedPos = gameObject->transform.GetGlobal().GetPositionFromTranslation();
			App::GetInstance()->GetSoundEngine().SetPosition(sound, cachedPos);
		}
	}

	void SoundPlayerComponent::Serialize(Core::Serialization::Serializer& sr) const
	{
		sr.Write(sound ? sound->hash : (u64)0);
	}

	void SoundPlayerComponent::Deserialize(Core::Serialization::Deserializer& dr)
	{
		u64 hash;
		dr.Read(hash);
		auto& res = App::GetInstance()->GetResources();
		sound = res.Get<Resources::Sound>(hash);
	}

	void SoundPlayerComponent::PlaySound()
	{
		if (sound) App::GetInstance()->GetSoundEngine().StartSound(sound);
	}

	void SoundPlayerComponent::StopSound()
	{
		if (sound) App::GetInstance()->GetSoundEngine().StopSound(sound);
	}

	void SoundPlayerComponent::Delete()
	{
		App::GetInstance()->GetResources().Release(sound);
	}
}