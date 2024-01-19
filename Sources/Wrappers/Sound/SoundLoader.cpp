#include "Wrappers/Sound/SoundLoader.hpp"

#include <filesystem>

#include "Core/App.hpp"

#include <STB_vorbis/stb_vorbis.h>
#include <miniaudio/miniaudio.h>

#include "Core/Debugging/Log.hpp"

#include "Wrappers/Sound/SoundEngine.hpp"
#include "Resources/Sound.hpp"
#include "Resources/SoundData.hpp"
#include "Core/Serialization/Conversion.hpp"
#include "Core/FileManager.hpp"

using namespace Wrappers::Sound;

Wrappers::Sound::SoundLoader::SoundLoader(ma_engine* soundEngine) : engine(soundEngine)
{
}

ma_result vfs_open(ma_vfs* pVFS, const char* pFilePath, ma_uint32 openMode, ma_vfs_file* pFile);
ma_result vfs_open_w(ma_vfs* pVFS, const wchar_t* pFilePath, ma_uint32 openMode, ma_vfs_file* pFile);
ma_result vfs_close(ma_vfs* pVFS, ma_vfs_file file);
ma_result vfs_read(ma_vfs* pVFS, ma_vfs_file file, void* pDst, size_t sizeInBytes, size_t* pBytesRead);
ma_result vfs_write(ma_vfs* pVFS, ma_vfs_file file, const void* pSrc, size_t sizeInBytes, size_t* pBytesWritten);
ma_result vfs_seek(ma_vfs* pVFS, ma_vfs_file file, ma_int64 offset, ma_seek_origin origin);
ma_result vfs_tell(ma_vfs* pVFS, ma_vfs_file file, ma_int64* pCursor);
ma_result vfs_info(ma_vfs* pVFS, ma_vfs_file file, ma_file_info* pInfo);

void SoundLoader::VFSInit(void* pVFS)
{
    ma_vfs_callbacks* callbacks = static_cast<ma_vfs_callbacks*>(pVFS);
    callbacks->onOpen = vfs_open;
    callbacks->onOpenW = vfs_open_w;
    callbacks->onClose = vfs_close;
    callbacks->onRead = vfs_read;
    callbacks->onWrite = vfs_write;
    callbacks->onSeek = vfs_seek;
    callbacks->onTell = vfs_tell;
    callbacks->onInfo = vfs_info;
}

Resources::SoundData* SoundLoader::CreateSoundAsset(const char* path)
{
    if (!Core::FileManager::FileExist(path))
    {
        LOG(DEBUG_LEVEL::LERROR, "File %s does not exist !", path);
        return nullptr;
    }
    auto data = Core::FileManager::LoadFile(path);
    std::filesystem::path p(path);
    Resources::SoundData* sound = Core::App::GetInstance()->GetResources().CreateResource<Resources::SoundData>(p.filename().string());
    sound->soundData = data;
    sound->isSaved = false;
    sound->isLoaded = true;
    return sound;
}

bool SoundLoader::LoadSound(Resources::Sound* sound)
{
    ma_result result;
    result = ma_sound_init_from_file(engine, Maths::Util::GetHex(sound->data->hash).c_str(), 0, NULL, NULL, &sound->sound);
    if (result != MA_SUCCESS)
    {
        LOG(DEBUG_LEVEL::LERROR, "Failed to load sound %s with error %s!", sound->path.c_str(), SoundEngine::GetSoundErrorCode(result));
        return false;
    }
    return true;
}

ma_result vfs_open_base(ma_vfs* pVFS, std::filesystem::path p, ma_uint32 openMode, ma_vfs_file* pFile, bool isWide)
{
    if (openMode == 0) return MA_INVALID_ARGS;
    u64 hash = Maths::Util::ReadHex(p.filename().string());
    SoundFile* file = new SoundFile();
    if (openMode & MA_OPEN_MODE_READ)
    {
        file->file = Core::App::GetInstance()->GetResources().Get<Resources::SoundData>(hash);
        if (!file->file) return MA_ERROR;
    }
    else
    {
        file->file = Core::App::GetInstance()->GetResources().CreateResource<Resources::SoundData>(p.filename().string());
        if (!file->file) return MA_ERROR;
    }

    *reinterpret_cast<SoundFile**>(pFile) = file;
    return MA_SUCCESS;
}

ma_result vfs_open(ma_vfs* pVFS, const char* pFilePath, ma_uint32 openMode, ma_vfs_file* pFile)
{
    std::filesystem::path p(pFilePath);
    return vfs_open_base(pVFS, p, openMode, pFile, false);
}

ma_result vfs_open_w(ma_vfs* pVFS, const wchar_t* pFilePath, ma_uint32 openMode, ma_vfs_file* pFile)
{
    std::filesystem::path p(pFilePath);
    return vfs_open_base(pVFS, p, openMode, pFile, true);
}

ma_result vfs_close(ma_vfs* pVFS, ma_vfs_file file)
{
    SoundFile* data = reinterpret_cast<SoundFile*>(file);
    if (!data->file->isSaved) Core::App::GetInstance()->GetResources().SaveAsset(data->file);
    delete data;
    return MA_SUCCESS;
}

ma_result vfs_read(ma_vfs* pVFS, ma_vfs_file file, void* pDst, size_t sizeInBytes, size_t* pBytesRead)
{
    SoundFile* data = reinterpret_cast<SoundFile*>(file);
    if (!data) return MA_ERROR;
    u64 len;
    if (data->filePos + sizeInBytes > data->file->soundData.size())
    {
        len = data->file->soundData.size() - data->filePos;
    }
    else
    {
        len = sizeInBytes;
    }
    std::copy(data->file->soundData.c_str() + data->filePos, data->file->soundData.c_str() + data->filePos + len, reinterpret_cast<char*>(pDst));
    data->filePos += len;
    if (pBytesRead != NULL)
    {
        *pBytesRead = len;
    }
    if (!len) return MA_AT_END;

    return MA_SUCCESS;
}

ma_result vfs_write(ma_vfs* pVFS, ma_vfs_file file, const void* pSrc, size_t sizeInBytes, size_t* pBytesWritten)
{
    SoundFile* data = reinterpret_cast<SoundFile*>(file);
    if (!data) return MA_ERROR;
    data->file->soundData.resize(data->filePos + sizeInBytes);
    std::copy(reinterpret_cast<const char*>(pSrc), reinterpret_cast<const char*>(pSrc) + sizeInBytes, data->file->soundData.data() + data->filePos);
    data->filePos += sizeInBytes;
    if (data->file->soundData.size()) data->file->isLoaded = true;
    if (pBytesWritten != NULL)
    {
        *pBytesWritten = sizeInBytes;
    }
    data->file->isSaved = false;
    return MA_SUCCESS;
}

ma_result vfs_seek(ma_vfs* pVFS, ma_vfs_file file, ma_int64 offset, ma_seek_origin origin)
{
    SoundFile* data = reinterpret_cast<SoundFile*>(file);
    if (!data) return MA_ERROR;
    u64 begin;
    switch (origin)
    {
    case ma_seek_origin_current:
        begin = data->filePos;
        break;
    case ma_seek_origin_end:
        begin = data->file->soundData.size();
        break;
    default:
        begin = 0;
        break;
    }
    s64 pos = begin + offset;
    if (offset < 0 || static_cast<u64>(offset) > data->file->soundData.size()) return MA_INVALID_ARGS;
    data->filePos = pos;

    return MA_SUCCESS;
}

ma_result vfs_tell(ma_vfs* pVFS, ma_vfs_file file, ma_int64* pCursor)
{
    SoundFile* data = reinterpret_cast<SoundFile*>(file);
    if (!data) return MA_ERROR;
    if (!pCursor) return MA_INVALID_ARGS;
    *pCursor = data->filePos;

    return MA_SUCCESS;
}

ma_result vfs_info(ma_vfs* pVFS, ma_vfs_file file, ma_file_info* pInfo)
{
    SoundFile* data = reinterpret_cast<SoundFile*>(file);
    if (!data) return MA_ERROR;
    if (!pInfo) return MA_INVALID_ARGS;
    pInfo->sizeInBytes = data->file->soundData.size();

    return MA_SUCCESS;
}

void Wrappers::Sound::SoundLoader::UnLoadSound(Resources::Sound* sound)
{
    ma_sound_uninit(&sound->sound);
}
