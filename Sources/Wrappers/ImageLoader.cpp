#include "Wrappers/ImageLoader.hpp"

#include <filesystem>

#include "Core/Debugging/Log.hpp"
#include "Core/FileManager.hpp"

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <STB_Image/stb_image.h>
#include "STB_Image/stb_image_write.h"

namespace Wrappers
{
	const char* ImageLoader::GetLastError()
	{
		return stbi_failure_reason();
	}

	u8* ImageLoader::LoadStbi(char const* filename, s32* x, s32* y, s32* comp)
	{
		return stbi_load(filename, x, y, comp, STBI_rgb_alpha);
	}
	
	f32* ImageLoader::LoadStbiFloat(char const* filename, s32* x, s32* y, s32* comp, bool gammaCorrection)
	{
		f32* result = stbi_loadf(filename, x, y, comp, STBI_rgb_alpha);
		if (!gammaCorrection)
		{
			for (u64 i = 0; i < 1llu * (*x) * (*y); i++)
			{
				result[i    ] = powf(result[i    ], 1 / 2.2451125f);
				result[i + 1] = powf(result[i + 1], 1 / 2.2451125f);
				result[i + 2] = powf(result[i + 2], 1 / 2.2451125f);
				result[i + 3] = powf(result[i + 3], 1 / 2.2451125f);
			}
		}
		return result;
	}

	void ImageLoader::FreeStbi(void* retval_from_stbi_load)
	{
		free(retval_from_stbi_load);
	}

	bool ImageLoader::WriteStbi(char const* filename, s32 w, s32 h, s32 comp, const u8* data, s32 stride_in_bytes)
	{
		return stbi_write_png(filename, w, h, comp, data, stride_in_bytes) != 0;
	}

	bool ImageLoader::WriteStbi(char const* filename, s32 w, s32 h, s32 comp, const f32* data, bool gammaCorrection)
	{
		if (gammaCorrection)
		{
			return stbi_write_hdr(filename, w, h, comp, data) != 0;
		}
		else
		{
			f32* buffer = new f32[4llu * w * h];
			for (u64 i = 0; i < 4llu * w * h; i += 4)
			{
				buffer[i    ] = powf(data[i    ], 2.2451125f);
				buffer[i + 1] = powf(data[i + 1], 2.2451125f);
				buffer[i + 2] = powf(data[i + 2], 2.2451125f);
				buffer[i + 3] = powf(data[i + 3], 2.2451125f);
			}
			int result = stbi_write_hdr(filename, w, h, comp, buffer) != 0;
			delete[] buffer;
			return result;
		}
	}

	bool ImageLoader::ParseTexture(char const* pFilePath, Resources::StaticTexture* pOutput, TextureLoadingArg args)
	{
		pOutput->DeleteTextureData();

		s32 texChannels;
		switch (args)
		{
		case TextureLoadingArg::DEFAULT:
			pOutput->textureData = ImageLoader::LoadStbi(pFilePath, &pOutput->resolution.x, &pOutput->resolution.y, &texChannels);
			pOutput->isFloat = false;
			break;
		case TextureLoadingArg::SRGB:
			pOutput->textureData = ImageLoader::LoadStbiFloat(pFilePath, &pOutput->resolution.x, &pOutput->resolution.y, &texChannels, true);
			pOutput->isFloat = true;
			break;
		case TextureLoadingArg::SRGB_LINEAR:
			pOutput->textureData = ImageLoader::LoadStbiFloat(pFilePath, &pOutput->resolution.x, &pOutput->resolution.y, &texChannels, false);
			pOutput->isFloat = true;
			break;
		default:
			break;
		}
		if (!pOutput->textureData || pOutput->resolution.x <= 0 || pOutput->resolution.y <= 0)
		{
			LOG(DEBUG_LEVEL::LERROR, "Failed to load texture %s !", pFilePath);
			LOG(DEBUG_LEVEL::LERROR, ImageLoader::GetLastError());
			return false;
		}

		pOutput->UpdateMipLevel();
		return true;
	}

	bool ImageLoader::ParseCubeMap(char const* pFilePath, Resources::StaticCubeMap* pOutput)
	{
		pOutput->DeleteCubeMapData();
		LOG(DEBUG_LEVEL::LINFO, "Loading CubeMap %s", pFilePath);
		auto file = Core::FileManager::LoadFile(pFilePath);
		if (file.empty())
		{
			LOG(DEBUG_LEVEL::LERROR, "Could not open CubeMap %s !", pOutput);
			return false;
		}
		const std::string fileDir = std::filesystem::path(pFilePath).parent_path().string() + "/";
		std::string texPath = pFilePath;
		size_t decal = texPath.find_last_of('/') + 1;
		texPath = texPath.substr(0, decal);
		std::istringstream st(file);
		std::string line;
		TextureLoadingArg args = TextureLoadingArg::DEFAULT;
		std::vector<void*> data;
		Maths::IVec2 lastSize;
		auto DeleteData = [&]()
		{
			for (auto& ptr : data)
			{
				if (ptr) FreeStbi(ptr);
				data.clear();
			}
		};
		auto CheckArg = [&](const std::string& line) -> bool
		{
			if (data.size())
			{
				LOG(DEBUG_LEVEL::LWARNING, "Found Texture specifier %s for CubeMap %s after texture path; specifier will be ignored", line.c_str(), pFilePath);
				LOG(DEBUG_LEVEL::LWARNING, "Texture specifier(s) must be declared before any texture path");
				return true;
			}
			return false;
		};
		while (std::getline(st, line))
		{
			if (line.back() == '\r') line.pop_back();
			if (line.empty()) continue;
			if (line == "#SRGB")
			{
				if (CheckArg(line)) continue;
				args = TextureLoadingArg::SRGB;
				continue;
			}
			if (line == "#SRGB_LINEAR")
			{
				if (CheckArg(line)) continue;
				args = TextureLoadingArg::SRGB_LINEAR;
				continue;
			}
			if (line[0] == '#') continue;
			if (data.size() >= 6)
			{
				LOG(DEBUG_LEVEL::LWARNING, "Extra data found in CubeMap file %s", pFilePath);
				break;
			}
			std::string texFile = fileDir + line;
			LOG(DEBUG_LEVEL::LINFO, "Trying to load Texture %s", texFile.c_str());
			s32 texChannels;
			lastSize = pOutput->resolution;
			switch (args)
			{
			case TextureLoadingArg::DEFAULT:
				data.push_back(ImageLoader::LoadStbi(texFile.c_str(), &pOutput->resolution.x, &pOutput->resolution.y, &texChannels));
				break;
			case TextureLoadingArg::SRGB:
				data.push_back(ImageLoader::LoadStbiFloat(texFile.c_str(), &pOutput->resolution.x, &pOutput->resolution.y, &texChannels, true));
				break;
			case TextureLoadingArg::SRGB_LINEAR:
				data.push_back(ImageLoader::LoadStbiFloat(texFile.c_str(), &pOutput->resolution.x, &pOutput->resolution.y, &texChannels, false));
				break;
			default:
				break;
			}
			if (!data.back() || pOutput->resolution.x <= 0 || pOutput->resolution.y <= 0)
			{
				LOG(DEBUG_LEVEL::LERROR, "Failed to load texture %s for CubeMap %s !", texFile.c_str(), pFilePath);
				LOG(DEBUG_LEVEL::LERROR, ImageLoader::GetLastError());
				DeleteData();
				return false;
			}
			else if (lastSize.x != 0 && lastSize != pOutput->resolution)
			{
				LOG(DEBUG_LEVEL::LERROR, "Failed to load texture %s for CubeMap %s : All faces of the CubeMap must have the same resolution !", texFile.c_str(), pFilePath);
				LOG(DEBUG_LEVEL::LERROR, "Previous size was %s, new size is %s", lastSize.toString().c_str(), pOutput->resolution.toString().c_str());
				DeleteData();
				return false;
			}
		}
		if (data.size() != 6)
		{
			LOG(DEBUG_LEVEL::LERROR, "Invalid texture count in CubeMap %s : 6 expected, %d found", pFilePath, data.size());
			DeleteData();
			return false;
		}
		pOutput->isFloat = args != TextureLoadingArg::DEFAULT;
		u64 faceLen = 4llu * pOutput->resolution.x * pOutput->resolution.y;
		u64 dataLen = 6 * faceLen;
		if (pOutput->isFloat)
		{
			pOutput->cubeMapData = malloc(sizeof(f32) * dataLen);
			for (u8 i = 0; i < 6; i++)
			{
				std::copy(reinterpret_cast<f32*>(data[i]), reinterpret_cast<f32*>(data[i]) + faceLen, reinterpret_cast<f32*>(pOutput->cubeMapData) + i * faceLen);
			}
			
		}
		else
		{
			pOutput->cubeMapData = malloc(dataLen);
			for (u8 i = 0; i < 6; i++)
			{
				std::copy(reinterpret_cast<u8*>(data[i]), reinterpret_cast<u8*>(data[i]) + faceLen, reinterpret_cast<u8*>(pOutput->cubeMapData) + i * faceLen);
			}

		}
		pOutput->UpdateMipLevel();
		return true;
	}
}