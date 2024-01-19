#pragma once

#include <string>

#include "Core/Types.hpp"

namespace Resources
{
	class Sound;
	class SoundData;
}

struct ma_engine;

namespace Wrappers::Sound
{
	struct SoundFile
	{
		Resources::SoundData* file = nullptr;
		u64 filePos = 0;
	};
	class SoundLoader
	{
	public:
		SoundLoader(ma_engine* soundEngine);

		~SoundLoader() = default;

		Resources::SoundData* CreateSoundAsset(const char* path);
		bool LoadSound(Resources::Sound* sound);
		void UnLoadSound(Resources::Sound* sound);
		void VFSInit(void* pVFS);

	private:
		ma_engine* engine = nullptr;
	};

}