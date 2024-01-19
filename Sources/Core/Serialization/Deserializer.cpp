#include "Core/Serialization/Deserializer.hpp"

using namespace Core::Serialization;

bool Deserializer::Read(u8& in)
{
	if (cPos >= bufferSize) return false;
	in = buffer[cPos++];
	return true;
}

bool Deserializer::Read(s8& in)
{
	return Read(reinterpret_cast<u8&>(in));
}

bool Deserializer::Read(u16& in)
{
	u16 tmp = 0;
	if (cPos + sizeof(tmp) > bufferSize) return false;
	for (u8 i = 0; i < sizeof(tmp); i++)
	{
		tmp |= (u64)buffer[cPos++] << (i*8);
	}
	Conversion::ToLocal(tmp, in);
	return true;
}

bool Deserializer::Read(s16& in)
{
	return Read(reinterpret_cast<u16&>(in));
}

bool Deserializer::Read(u32& in)
{
	u32 tmp = 0;
	if (cPos + sizeof(tmp) > bufferSize) return false;
	for (u8 i = 0; i < sizeof(tmp); i++)
	{
		tmp |= (u64)buffer[cPos++] << (i * 8);
	}
	Conversion::ToLocal(tmp, in);
	return true;
}

bool Deserializer::Read(s32& in)
{
	return Read(reinterpret_cast<u32&>(in));
}

bool Deserializer::Read(u64& in)
{
	u64 tmp = 0;
	if (cPos + sizeof(tmp) > bufferSize) return false;
	for (u8 i = 0; i < sizeof(tmp); i++)
	{
		tmp |= (u64)buffer[cPos++] << (i * 8);
	}
	Conversion::ToLocal(tmp, in);
	return true;
}

bool Deserializer::Read(s64& in)
{
	return Read(reinterpret_cast<u64&>(in));
}

bool Deserializer::Read(f32& in)
{
	u32 tmp = 0;
	if (cPos + sizeof(tmp) > bufferSize) return false;
	for (u8 i = 0; i < sizeof(tmp); i++)
	{
		tmp |= (u64)buffer[cPos++] << (i * 8);
	}
	Conversion::ToLocal(tmp, in);
	return true;
}

bool Deserializer::Read(f64& in)
{
	u64 tmp = 0;
	if (cPos + sizeof(tmp) > bufferSize) return false;
	for (u8 i = 0; i < sizeof(tmp); i++)
	{
		tmp |= (u64)buffer[cPos++] << (i * 8);
	}
	Conversion::ToLocal(tmp, in);
	return true;
}

bool Deserializer::Read(bool& in)
{
	return Read(reinterpret_cast<u8&>(in));
}

bool Deserializer::Read(u8* dataIn, u64 dataSize)
{
	if (cPos + dataSize > bufferSize) return false;
	std::copy(buffer + cPos, buffer + (cPos + dataSize), dataIn);
	cPos += dataSize;
	return true;
}

bool Deserializer::Read(std::string& in)
{
	u64 size;
	if (!Read(size)) return false;
	assert(size + cPos <= bufferSize);
	in.resize(size);
	return Read(reinterpret_cast<u8*>(in.data()), size);
}

bool Deserializer::Read(Maths::Vec2& in)
{
	return Read(in.x) && Read(in.y);
}

bool Deserializer::Read(Maths::Vec3& in)
{
	return Read(in.x) && Read(in.y) && Read(in.z);
}

bool Deserializer::Read(Maths::Vec4& in)
{
	return Read(in.x) && Read(in.y) && Read(in.z) && Read(in.w);
}

bool Deserializer::Read(Maths::IVec2& in)
{
	return Read(in.x) && Read(in.y);
}

bool Deserializer::Read(Maths::IVec3& in)
{
	return Read(in.x) && Read(in.y) && Read(in.z);
}

bool Deserializer::Read(Maths::Quat& in)
{
	return Read(in.v.x) && Read(in.v.y) && Read(in.v.z) && Read(in.a);
}

bool Deserializer::Read(Maths::AABB& in)
{
	return Read(in.center) && Read(in.size);
}

bool Deserializer::Read(Maths::Frustum& in)
{
	return Read(in.top) && Read(in.bottom) && Read(in.right) && Read(in.left) && Read(in.front) && Read(in.back);
}