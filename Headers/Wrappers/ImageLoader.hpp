#pragma once

#include "Core/Types.hpp"

#include "Resources/StaticTexture.hpp"
#include "Resources/StaticCubeMap.hpp"

namespace Wrappers
{
	enum class TextureLoadingArg : u8
	{
		DEFAULT = 0,
		SRGB,
		SRGB_LINEAR,
	};
	class NAT_API ImageLoader
	{
	public:
		ImageLoader() = default;
		~ImageLoader() = default;

		static const char* GetLastError();
		static u8* LoadStbi(char const* filename, s32* x, s32* y, s32* comp);
		static f32* LoadStbiFloat(char const* filename, s32* x, s32* y, s32* comp, bool gammaCorrection);
		static void FreeStbi(void* retval_from_stbi_load);
		static bool WriteStbi(char const* filename, s32 w, s32 h, s32 comp, const u8* data, s32 stride_in_bytes = 0);
		static bool WriteStbi(char const* filename, s32 w, s32 h, s32 comp, const f32* data, bool gammaCorrection);

		static bool ParseTexture(char const* pFilePath, Resources::StaticTexture* pOutput, TextureLoadingArg args = TextureLoadingArg::DEFAULT);
		static bool ParseCubeMap(char const* pFilePath, Resources::StaticCubeMap* pOutput);
	};
}
