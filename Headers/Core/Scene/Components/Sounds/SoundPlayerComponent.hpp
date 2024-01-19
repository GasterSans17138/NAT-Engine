#pragma once
#include "Core/Scene/Components/IComponent.hpp"

namespace Resources
{
	class Sound;
}

namespace Core::Scene::Components::Sounds
{
	class NAT_API SoundPlayerComponent : public IComponent
	{
	public:
		SoundPlayerComponent() = default;
		~SoundPlayerComponent() = default;

		void Init();

		IComponent* CreateCopy() override;
		void RenderGui() override;
		ComponentType GetType() override;
		const char* GetName() override;
		void Update() override;
		void DataUpdate() override;
		void Serialize(Core::Serialization::Serializer& sr) const override;
		void Deserialize(Core::Serialization::Deserializer& dr) override;
		virtual void Delete() override;

		void PlaySound();
		void StopSound();

	public:
		Resources::Sound* sound = nullptr;
		Maths::Vec3 cachedPos;
	private:

	};

}