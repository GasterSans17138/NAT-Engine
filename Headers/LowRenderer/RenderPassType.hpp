#pragma once

#include "Core/Types.hpp"

namespace LowRenderer
{
	enum RenderPassType : u32
	{
		DEFAULT   = 0x001,
		SHADOWMAP = 0x002,
		CUBEMAP   = 0x004,
		SECONDARY = 0x008,
		OBJECT    = 0x010,
		HALO      = 0x020,
		LIGHT     = 0x040,
		POST      = 0x080,
		WIRE      = 0x100,
		DEPTH     = 0x200,
		LINE      = 0x400,
		ALL       =   ~0U,
	};
}