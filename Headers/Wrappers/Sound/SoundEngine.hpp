#pragma once

#include "Core/Types.hpp"
#include "Maths/Maths.hpp"

#include <MiniAudio/miniaudio.h>

namespace Resources
{
	class Sound;
}

namespace Wrappers
{
	class Interfacing;

	namespace Sound
	{
		enum class AttenuationType : u8
		{
			NONE = 0,
			INVERSE,
			LINEAR,
			EXPONENTIAL,
		};
		class NAT_API SoundEngine
		{
		public:
			SoundEngine() = default;
			~SoundEngine() = default;
			void Init();
			void Shutdown();

			void SelectSoundDevice();

			static const char* GetSoundErrorCode(s32 code);

			void Update(Interfacing* guiInterface);

			ma_engine* GetEngine();

			void StartSound(Resources::Sound* pSound);
			void StopSound(Resources::Sound* pSound);
			void ResetSound(Resources::Sound* pSound);

			void SetVolumeLinear(Resources::Sound* pSound, f32 pVolume);
			void SetVolumeDecibels(Resources::Sound* pSound, f32 pDecibels);
			void SetPan(Resources::Sound* pSound, f32 pPan);
			void SetPitch(Resources::Sound* pSound, f32 pPitch);
			void SetLooping(Resources::Sound* pSound, bool looping);
			void SetSpatializationEnabled(Resources::Sound* pSound, bool active);
			void SetDopplerFactor(Resources::Sound* pSound, f32 pFactor);
			void SetRollOff(Resources::Sound* pSound, f32 pFactor);
			void SetAttenuationType(Resources::Sound* pSound, AttenuationType pType);
			void SetDistances(Resources::Sound* pSound, Maths::Vec2 bounds);
			void SetGains(Resources::Sound* pSound, Maths::Vec2 bounds);
			void SetPosition(Resources::Sound* pSound, Maths::Vec3 pos);
			void SetVelocity(Resources::Sound* pSound, Maths::Vec3 vel);
			void UpdateListener(Maths::Vec3 position, Maths::Vec3 up, Maths::Vec3 front, Maths::Vec3 velocity);

			bool IsPlaying(Resources::Sound* pSound);
			bool IsLooping(Resources::Sound* pSound);
			bool IsSpatializationEnabled(Resources::Sound* pSound);

			f32 GetVolumeLinear(const Resources::Sound* pSound);
			f32 GetVolumeDecibels(const Resources::Sound* pSound);
			f32 GetPan(const Resources::Sound* pSound);
			f32 GetPitch(const Resources::Sound* pSound);
			f32 GetDopplerFactor(const Resources::Sound* pSound);
			f32 GetRollOff(Resources::Sound* pSound);
			AttenuationType GetAttenuationType(Resources::Sound* pSound);
			Maths::Vec2 GetDistances(Resources::Sound* pSound);
			Maths::Vec2 GetGains(Resources::Sound* pSound);

		private:
			ma_engine engine = {};
			ma_device device = {};
			ma_resource_manager soundManager = {};
			ma_context context = {};
			ma_resource_manager_config soundManagerConfig = {};
			ma_device_info* pPlaybackDeviceInfos = {};
			ma_vfs* vfs = nullptr;
		};
	}
}