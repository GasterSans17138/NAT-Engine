#pragma once

#include "Maths/Maths.hpp"

#ifdef NAT_EngineDLL
#define NAT_API __declspec(dllexport)
#else
#define NAT_API __declspec(dllimport)
#endif // NAT_EngineDLL

namespace Core::Scene
{
	class GameObject;

	class NAT_API Transform
	{
	public:
		Transform(GameObject* pGameObject = nullptr);
		~Transform() = default;

		void Update(); //Update both Mat4

		const Maths::Mat4& GetLocal() const;
		const Maths::Mat4& GetGlobal() const;

		const Maths::Vec3& GetPosition() const;
		void SetPosition(const Maths::Vec3& pos);
		const Maths::Quat& GetRotation() const;
		void SetRotation(const Maths::Quat& rot);
		const Maths::Vec3& GetScale() const;
		void SetScale(const Maths::Vec3& sc);

	private:
		Maths::Vec3 position;
		Maths::Quat rotation;
		Maths::Vec3 scale = Maths::Vec3(1);

		Maths::Mat4 local = Maths::Mat4::Identity();
		Maths::Mat4 global = Maths::Mat4::Identity();

		GameObject* gameObject = nullptr;

		friend GameObject;
	};
}