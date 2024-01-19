#include "Core/Serialization/Serializer.hpp"

using namespace Core::Serialization;

void Serializer::Write(u8 in)
{
	buffer.push_back(in);
}

void Serializer::Write(s8 in)
{
	buffer.push_back(static_cast<u8>(in));
}

void Serializer::Write(u16 in)
{
	u16 tmp;
	Conversion::ToNetwork(in, tmp);
	for (u8 i = 0; i < sizeof(tmp); i++)
	{
		buffer.push_back((tmp >> (i*8) & 0xff));
	}
}

void Serializer::Write(s16 in)
{
	Write(static_cast<u16>(in));
}

void Serializer::Write(u32 in)
{
	u32 tmp;
	Conversion::ToNetwork(in, tmp);
	for (u8 i = 0; i < sizeof(tmp); i++)
	{
		buffer.push_back((tmp >> (i * 8) & 0xff));
	}
}

void Serializer::Write(s32 in)
{
	Write(static_cast<u32>(in));
}

void Serializer::Write(u64 in)
{
	u64 tmp;
	Conversion::ToNetwork(in, tmp);
	for (u8 i = 0; i < sizeof(tmp); i++)
	{
		buffer.push_back((tmp >> (i * 8) & 0xff));
	}
}

void Serializer::Write(s64 in)
{
	Write(static_cast<u64>(in));
}

void Serializer::Write(f32 in)
{
	u32 tmp;
	Conversion::ToNetwork(in, tmp);
	for (u8 i = 0; i < sizeof(tmp); i++)
	{
		buffer.push_back((tmp >> (i * 8) & 0xff));
	}
}

void Serializer::Write(f64 in)
{
	u64 tmp;
	Conversion::ToNetwork(in, tmp);
	for (u8 i = 0; i < sizeof(tmp); i++)
	{
		buffer.push_back((tmp >> (i * 8) & 0xff));
	}
}

void Serializer::Write(bool in)
{
	Write(static_cast<u8>(in));
}

void Serializer::Write(const u8* dataIn, u64 dataSize)
{
	buffer.resize(buffer.size() + dataSize);
	std::copy(dataIn, dataIn + dataSize, buffer.data() + (buffer.size() - dataSize));
}

void Serializer::Write(const std::string& in, bool writeSize)
{
	if (writeSize) Write(in.size());
	Write(reinterpret_cast<const u8*>(in.data()), in.size());
}

void Serializer::Write(Maths::Vec2 in)
{
	Write(in.x);
	Write(in.y);
}

void Serializer::Write(const Maths::Vec3& in)
{
	Write(in.x);
	Write(in.y);
	Write(in.z);
}

void Serializer::Write(const Maths::Vec4& in)
{
	Write(in.x);
	Write(in.y);
	Write(in.z);
	Write(in.w);
}

void Serializer::Write(Maths::IVec2 in)
{
	Write(in.x);
	Write(in.y);
}

void Serializer::Write(const Maths::IVec3& in)
{
	Write(in.x);
	Write(in.y);
	Write(in.z);
}

void Serializer::Write(const Maths::Quat& in)
{
	Write(in.v.x);
	Write(in.v.y);
	Write(in.v.z);
	Write(in.a);
}

void Serializer::Write(const Maths::AABB& in)
{
	Write(in.center);
	Write(in.size);
}

void Serializer::Write(const Maths::Frustum& in)
{
	Write(in.top);
	Write(in.bottom);
	Write(in.right);
	Write(in.left);
	Write(in.front);
	Write(in.back);
}