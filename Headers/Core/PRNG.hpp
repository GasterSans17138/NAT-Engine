#pragma once

#include <random>

#include "Types.hpp"

#ifdef NAT_EngineDLL
	#define NAT_API __declspec(dllexport)
#else
	#define NAT_API __declspec(dllimport)
#endif // NAT_EngineDLL

#pragma warning(disable:4251)

namespace Core
{
	class NAT_API PRNG
	{
	public:
		PRNG();
		PRNG(u32 seed);

		u64 NextLong();
		u64 NextLong(u64 min, u64 max);
		u32 NextInt();
		s32 NextInt(s32 min, s32 max);
		f32 NextFloat();
		f32 NextFloat(f32 min, f32 max);
		f64 NextDouble();
		f64 NextDouble(f64 min, f64 max);
	private:
		std::mt19937 mtw;
	};
}