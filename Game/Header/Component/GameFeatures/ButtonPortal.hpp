#pragma once
#include "../Game/Header/Component/GameFeatures/Button.hpp"



namespace Core::Scene::Components::Game
{

	class NAT_API ButtonPortal : public Button
	{
	public:
		ButtonPortal();
		~ButtonPortal();

		void Init() override;
		void Update() override;
		IComponent* CreateCopy() override;
		void RenderGui() override;
		ComponentType GetType() override;

		void Serialize(Core::Serialization::Serializer& sr) const override;
		void Deserialize(Core::Serialization::Deserializer& dr) override;
		const char* GetName() override;

		void OnClick() override;
		void SoundPlaying() override;

	private:
		Core::Scene::Components::Sounds::SoundPlayerComponent* soundPlayer = nullptr;
		GameObject* surfacec1 = nullptr;
		GameObject* surfacec2 = nullptr;
	};
}

