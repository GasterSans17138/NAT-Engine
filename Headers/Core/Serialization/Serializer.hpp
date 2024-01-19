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

#pragma warning(disable:4251)

namespace Core::Serialization
{
	class NAT_API Serializer
	{
	public:
		Serializer() = default;

		~Serializer() = default;

		const u8* GetBuffer() const { return buffer.data(); }
		const u64 GetBufferSize() const { return buffer.size(); }

		void Write(u8 in);
		void Write(s8 in);
		void Write(u16 in);
		void Write(s16 in);
		void Write(u32 in);
		void Write(s32 in);
		void Write(u64 in);
		void Write(s64 in);
		void Write(f32 in);
		void Write(f64 in);
		void Write(bool in);
		void Write(Maths::Vec2 in);
		void Write(const Maths::Vec3& in);
		void Write(const Maths::Vec4& in);
		void Write(Maths::IVec2 in);
		void Write(const Maths::IVec3& in);
		void Write(const Maths::Quat& in);
		void Write(const Maths::AABB& in);
		void Write(const Maths::Frustum& in);
		void Write(const u8* dataIn, u64 dataSize);
		void Write(const std::string& in, bool writeSize = true);
	private:
		std::vector<u8> buffer;
	};

}