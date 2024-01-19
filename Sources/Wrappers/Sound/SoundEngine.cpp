#include "Wrappers/Sound/SoundEngine.hpp"

#include "Core/App.hpp"

#include <STB_vorbis/stb_vorbis.h>
#define MINIAUDIO_IMPLEMENTATION
#include <MiniAudio/miniaudio.h>

#include "Core/Debugging/Log.hpp"
#include "Resources/Sound.hpp"
#include "Wrappers/Interfacing.hpp"

// TODO remove ?
#include "Core/Signal.hpp"
Core::Signal signal;
std::array<f32, 128> soundBuf;
Maths::Vec2 lastValue;
Maths::Vec2 prev;

using namespace Wrappers::Sound;

void SoundEngine::SelectSoundDevice()
{
}

const char* SoundEngine::GetSoundErrorCode(s32 code)
{
    return ma_result_description(static_cast<ma_result>(code));
}

void SoundEngine::Shutdown()
{
    ma_engine_uninit(&engine);
    ma_device_uninit(&device);
    ma_resource_manager_uninit(&soundManager);
    ma_context_uninit(&context);
    delete vfs;
}

void data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount)
{
    (void)pInput;
    ma_result result = ma_engine_read_pcm_frames(reinterpret_cast<SoundEngine*>(pDevice->pUserData)->GetEngine(), pOutput, frameCount, NULL);
    if (!signal())
    {
        f32* buf = static_cast<f32*>(pOutput);
        lastValue = Maths::Vec2();
        for (u32 i = 0; i < frameCount; ++i)
        {
            lastValue[0] += std::abs(buf[i*2llu] - prev[0]);
            lastValue[1] += std::abs(buf[i*2llu + 1] - prev[1]);
            prev = Maths::Vec2(buf[i*2llu], buf[i*2llu + 1]);
        }
        lastValue /= static_cast<f32>(frameCount);
        signal = true;
    }
}

void SoundEngine::Init()
{
    soundManagerConfig = ma_resource_manager_config_init();
    soundManagerConfig.decodedFormat = ma_format_f32;
    soundManagerConfig.decodedChannels = 2;
    soundManagerConfig.decodedSampleRate = 48000;
    vfs = malloc(sizeof(ma_vfs_callbacks));
    Core::App::GetInstance()->GetSoundLoader().VFSInit(vfs);
    soundManagerConfig.pVFS = vfs;
    ma_result result;
    result = ma_resource_manager_init(&soundManagerConfig, &soundManager);
    if (result != MA_SUCCESS)
    {
        LOG(DEBUG_LEVEL::LERROR, "Failed to initialize resource manager : %s", GetSoundErrorCode(result));
        return;
    }
    result = ma_context_init(NULL, 0, NULL, &context);
    if (result != MA_SUCCESS)
    {
        LOG(DEBUG_LEVEL::LERROR, "Failed to initialize context : %s", GetSoundErrorCode(result));
        return;
    }
    ma_uint32 deviceCount;
    result = ma_context_get_devices(&context, &pPlaybackDeviceInfos, &deviceCount, NULL, NULL);
    if (result != MA_SUCCESS)
    {
        LOG(DEBUG_LEVEL::LERROR, "Failed to enumerate playback devices : %s", GetSoundErrorCode(result));
        ma_context_uninit(&context);
        return;
    }
    ma_uint32 deviceID = 0;
    for (ma_uint32 i = 0; i < deviceCount; i++)
    {
        LOG(DEBUG_LEVEL::LDEFAULT, "Device %d : %s %s", i, (pPlaybackDeviceInfos)[i].name, (pPlaybackDeviceInfos)[i].isDefault ? "(Default Device)" : "");
        if ((pPlaybackDeviceInfos)[i].isDefault) deviceID = i;
    }
    ma_device_config deviceConfig;
    ma_engine_config engineConfig;

    deviceConfig = ma_device_config_init(ma_device_type_playback);
    deviceConfig.playback.pDeviceID = &pPlaybackDeviceInfos[deviceID].id;
    deviceConfig.playback.format = soundManager.config.decodedFormat;
    deviceConfig.playback.channels = 0;
    deviceConfig.sampleRate = soundManager.config.decodedSampleRate;
    deviceConfig.dataCallback = data_callback;
    deviceConfig.pUserData = this;
    result = ma_device_init(&context, &deviceConfig, &device);
    if (result != MA_SUCCESS)
    {
        LOG(DEBUG_LEVEL::LERROR, "Failed to initialize device %s : %s", (pPlaybackDeviceInfos)[deviceID].name, GetSoundErrorCode(result));
        return;
    }
    engineConfig = ma_engine_config_init();
    engineConfig.pDevice = &device;
    engineConfig.pResourceManager = &soundManager;
    engineConfig.noAutoStart = MA_TRUE;
    result = ma_engine_init(&engineConfig, &engine);
    if (result != MA_SUCCESS)
    {
        LOG(DEBUG_LEVEL::LERROR, "Failed to initialize engine with device %s : %s", &(pPlaybackDeviceInfos)[deviceID].name, GetSoundErrorCode(result));
        ma_device_uninit(&device);
        return;
    }
    result = ma_engine_start(&engine);
    if (result != MA_SUCCESS)
    {
        LOG(DEBUG_LEVEL::LWARNING, "Failed to start engine : %s", GetSoundErrorCode(result));
    }
}

void SoundEngine::Update(Interfacing* guiInterface)
{
    if (signal())
    {
        for (u8 i = 0; i < 64 - 1; ++i)
        {
            soundBuf[i] = soundBuf[i + 1llu];
            soundBuf[i + 64llu] = soundBuf[i + 65llu];
        }
        soundBuf[63] = lastValue[0];
        soundBuf[127] = lastValue[1];
        signal = false;
    }
    guiInterface->Begin("Sound Engine");
    guiInterface->PlotHistogram("Sound Buffer L", soundBuf.data(), 64, nullptr, 0.0f, 0.5f, Maths::Vec2(guiInterface->GetWindowSize().x - 10, 100));
    guiInterface->PlotHistogram("Sound Buffer R", soundBuf.data() + 64, 64, nullptr, 0.0f, 0.5f, Maths::Vec2(guiInterface->GetWindowSize().x - 10, 100));
    guiInterface->End();
}

ma_engine* SoundEngine::GetEngine()
{
    return &engine;
}

void SoundEngine::StartSound(Resources::Sound* pSound)
{
    ma_sound_set_start_time_in_pcm_frames(&pSound->sound, ma_engine_get_time_in_pcm_frames(&engine));
    ma_sound_start(&pSound->sound);
}

void SoundEngine::StopSound(Resources::Sound* pSound)
{
    ma_sound_stop(&pSound->sound);
}

void SoundEngine::ResetSound(Resources::Sound* pSound)
{
    ma_sound_seek_to_pcm_frame(&pSound->sound, 0);
}

void SoundEngine::SetVolumeLinear(Resources::Sound* pSound, f32 pVolume)
{
    ma_sound_set_volume(&pSound->sound, pVolume);
}

void SoundEngine::SetVolumeDecibels(Resources::Sound* pSound, f32 pDecibels)
{
    ma_sound_set_volume(&pSound->sound, ma_volume_db_to_linear(pDecibels));
}

void SoundEngine::SetPan(Resources::Sound* pSound, f32 pPan)
{
    ma_sound_set_pan(&pSound->sound, pPan);
}

void SoundEngine::SetPitch(Resources::Sound* pSound, f32 pPitch)
{
    ma_sound_set_pitch(&pSound->sound, pPitch);
}

void SoundEngine::SetDopplerFactor(Resources::Sound* pSound, f32 pFactor)
{
    ma_sound_set_doppler_factor(&pSound->sound, pFactor);
}

void SoundEngine::SetSpatializationEnabled(Resources::Sound* pSound, bool active)
{
    ma_sound_set_spatialization_enabled(&pSound->sound, active);
}

void SoundEngine::SetRollOff(Resources::Sound* pSound, f32 pFactor)
{
    ma_sound_set_rolloff(&pSound->sound, pFactor);
}

void SoundEngine::SetAttenuationType(Resources::Sound* pSound, AttenuationType pType)
{
    ma_sound_set_attenuation_model(&pSound->sound, (ma_attenuation_model)pType);
}

void SoundEngine::SetDistances(Resources::Sound* pSound, Maths::Vec2 bounds)
{
    ma_sound_set_min_distance(&pSound->sound, bounds.x);
    ma_sound_set_max_distance(&pSound->sound, bounds.y);
}

void SoundEngine::SetGains(Resources::Sound* pSound, Maths::Vec2 bounds)
{
    ma_sound_set_min_gain(&pSound->sound, bounds.x);
    ma_sound_set_max_gain(&pSound->sound, bounds.y);
}

void SoundEngine::SetPosition(Resources::Sound* pSound, Maths::Vec3 pos)
{
    ma_sound_set_position(&pSound->sound, pos.x, pos.y, pos.z);
}

void SoundEngine::SetVelocity(Resources::Sound* pSound, Maths::Vec3 vel)
{
    ma_sound_set_velocity(&pSound->sound, vel.x, vel.y, vel.z);
}

void SoundEngine::UpdateListener(Maths::Vec3 position, Maths::Vec3 up, Maths::Vec3 front, Maths::Vec3 velocity)
{
    ma_engine_listener_set_position(&engine, 0, position.x, position.y, position.z);
    ma_engine_listener_set_direction(&engine, 0, front.x, front.y, front.z);
    ma_engine_listener_set_world_up(&engine, 0, up.x, up.y, up.z);
    ma_engine_listener_set_velocity(&engine, 0, velocity.x, velocity.y, velocity.z);
}

void SoundEngine::SetLooping(Resources::Sound* pSound, bool looping)
{
    ma_sound_set_looping(&pSound->sound, looping);
}

bool SoundEngine::IsPlaying(Resources::Sound* pSound)
{
    return ma_sound_is_playing(&pSound->sound);
}

bool SoundEngine::IsLooping(Resources::Sound* pSound)
{
    return ma_sound_is_looping(&pSound->sound);
}

bool Wrappers::Sound::SoundEngine::IsSpatializationEnabled(Resources::Sound* pSound)
{
    return ma_sound_is_spatialization_enabled(&pSound->sound);
}

f32 SoundEngine::GetDopplerFactor(const Resources::Sound* pSound)
{
    return ma_sound_get_doppler_factor(&pSound->sound);
}

f32 SoundEngine::GetRollOff(Resources::Sound* pSound)
{
    return ma_sound_get_rolloff(&pSound->sound);
}

AttenuationType SoundEngine::GetAttenuationType(Resources::Sound* pSound)
{
    return static_cast<AttenuationType>(ma_sound_get_attenuation_model(&pSound->sound));
}

Maths::Vec2 SoundEngine::GetDistances(Resources::Sound* pSound)
{
    return Maths::Vec2(ma_sound_get_min_distance(&pSound->sound), ma_sound_get_max_distance(&pSound->sound));
}

Maths::Vec2 SoundEngine::GetGains(Resources::Sound* pSound)
{
    return Maths::Vec2(ma_sound_get_min_gain(&pSound->sound), ma_sound_get_max_gain(&pSound->sound));
}

f32 SoundEngine::GetVolumeLinear(const Resources::Sound* pSound)
{
    return ma_sound_get_volume(&pSound->sound);
}

f32 SoundEngine::GetVolumeDecibels(const Resources::Sound* pSound)
{
    return ma_volume_linear_to_db(ma_sound_get_volume(&pSound->sound));
}

f32 SoundEngine::GetPan(const Resources::Sound* pSound)
{
    return ma_sound_get_pan(&pSound->sound);
}

f32 SoundEngine::GetPitch(const Resources::Sound* pSound)
{
    return ma_sound_get_pitch(&pSound->sound);
}
