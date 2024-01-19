#pragma once

#include "IResource.hpp"
#include "Wrappers/Sound/SoundEngine.hpp"
#include "Wrappers/Sound/SoundLoader.hpp"
#include "ResourceManager.hpp"

namespace Resources
{
	class SoundData;

	class NAT_API Sound : public IResource
	{
	public:
		Sound();

		~Sound() override = default;

		void DeleteData() override;
		bool ShowEditWindow() override;
		void Write(Core::Serialization::Serializer& sr) override;
		void Load(Core::Serialization::Deserializer& dr) override;
		void WindowCreateResource(bool& open) override;
		ObjectType GetType() override { return ObjectType::SoundType; }

	private:
		SoundData* data = nullptr;
		ma_sound sound = {};
		bool prevbool = false;
		Sound* newSound = nullptr;

		friend Wrappers::Sound::SoundEngine;
		friend Wrappers::Sound::SoundLoader;
	};

}