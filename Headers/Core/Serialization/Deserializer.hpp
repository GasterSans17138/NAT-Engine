#pragma once

#include <vector>
#include <string>
#include "Conversion.hpp"
#include "Maths/Maths.hpp"

#ifdef NAT_EngineDLL
	#define NAT_API __declspec(dllexport)
#else
	#define NAT_API __declspec(dllimport)
#endif // NAT_EngineDLL

namespace Core::Serialization
{
	class NAT_API Deserializer
	{
	public:
		Deserializer(const u8* data, u64 dataSize) : buffer(data), bufferSize(dataSize) {}
		Deserializer(const std::vector<u8>& data) : Deserializer(data.data(), data.size()) {}

		~Deserializer() = default;

		const u64 CursorPos() const { return cPos; }
		const u64 BufferSize() const { return bufferSize; }

		bool Read(u8& in);
		bool Read(s8& in);
		bool Read(u16& in);
		bool Read(s16& in);
		bool Read(u32& in);
		bool Read(s32& in);
		bool Read(u64& in);
		bool Read(s64& in);
		bool Read(f32& in);
		bool Read(f64& in);
		bool Read(bool& in);
		bool Read(Maths::Vec2& in);
		bool Read(Maths::Vec3& in);
		bool Read(Maths::Vec4& in);
		bool Read(Maths::IVec2& in);
		bool Read(Maths::IVec3& in);
		bool Read(Maths::Quat& in);
		bool Read(Maths::AABB& in);
		bool Read(Maths::Frustum& in);
		bool Read(u8* dataIn, u64 dataSize);
		bool Read(std::string& in);
	private:
		const u8* buffer;
		const u64 bufferSize;
		u64 cPos = 0;
	};

}