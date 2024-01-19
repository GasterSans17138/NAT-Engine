#include "Core/PRNG.hpp"

#include <time.h>

std::uniform_int_distribution<uint32_t> uint_dist;
std::uniform_int_distribution<int32_t> int_dist;
std::uniform_int_distribution<uint64_t> uint64_dist;
std::normal_distribution<double> double_dist;
std::normal_distribution<float> float_dist;

using namespace Core;

PRNG::PRNG()
{
	mtw.seed((u32(time(nullptr))));
}

PRNG::PRNG(u32 seed)
{
	mtw.seed(seed);
}

u64 PRNG::NextLong()
{
	return uint64_dist(mtw);
}

u64 Core::PRNG::NextLong(u64 min, u64 max)
{
	std::uniform_int_distribution<u64> int_var_dist = std::uniform_int_distribution<u64>(min, max);
	return int_var_dist(mtw);
}

u32 PRNG::NextInt()
{
	return int_dist(mtw);
}

s32 PRNG::NextInt(s32 min, s32 max)
{
	std::uniform_int_distribution<s32> int_var_dist = std::uniform_int_distribution<s32>(min, max);
	return int_var_dist(mtw);
}

f64 PRNG::NextDouble()
{
	return double_dist(mtw);
}

f64 Core::PRNG::NextDouble(f64 min, f64 max)
{
	std::uniform_real_distribution<f64> float_var_dist = std::uniform_real_distribution<f64>(min, max);
	return float_var_dist(mtw);
}

f32 PRNG::NextFloat()
{
	return float_dist(mtw);
}

f32 Core::PRNG::NextFloat(f32 min, f32 max)
{
	std::uniform_real_distribution<f32> float_var_dist = std::uniform_real_distribution<f32>(min, max);
	return float_var_dist(mtw);
}
