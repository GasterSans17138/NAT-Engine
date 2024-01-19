#pragma once

#include "Core/Types.hpp"


#ifdef NAT_EngineDLL
	#define NAT_API __declspec(dllexport)
#else
	#define NAT_API __declspec(dllimport)
#endif // NAT_EngineDLL

namespace Core::Serialization::Conversion
{
	NAT_API void ToNetwork(u16 in, u16& out);
	NAT_API void ToNetwork(u32 in, u32& out);
	NAT_API void ToNetwork(u64 in, u64& out);
	NAT_API void ToNetwork(f32 in, u32& out);
	NAT_API void ToNetwork(f64 in, u64& out);

	NAT_API void ToLocal(u16 in, u16& out);
	NAT_API void ToLocal(u32 in, u32& out);
	NAT_API void ToLocal(u64 in, u64& out);
	NAT_API void ToLocal(u32 in, f32& out);
	NAT_API void ToLocal(u64 in, f64& out);
}