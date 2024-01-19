#include "Maths/Maths.hpp"

#include "Core/Scene/Transform.hpp"
#include "Core/Scene/GameObject.hpp"

namespace Core::Scene
{
	Transform::Transform(GameObject* pGameObject) : gameObject(pGameObject)
	{

	}

	void Transform::Update()
	{
		local = Maths::Mat4::CreateTransformMatrix(position, rotation, scale);

		if (!gameObject->parent)
			global = local;
		else
			global = gameObject->parent->transform.global * local;
	}
	 
	const Maths::Mat4& Transform::GetLocal() const
	{
		return local;
	}

	const Maths::Mat4& Transform::GetGlobal() const
	{
		return global;
	}

	const Maths::Vec3& Transform::GetPosition() const
	{
		return position;
	}

	void Transform::SetPosition(const Maths::Vec3& pos)
	{
		position = pos;
		Update();
		for (auto& comp : gameObject->components)
		{
			comp->OnPositionUpdate(global.GetPositionFromTranslation());
		}
	}

	const Maths::Quat& Transform::GetRotation() const
	{
		return rotation;
	}

	void Transform::SetRotation(const Maths::Quat& rot)
	{
		rotation = rot;
		Update();
		for (auto& comp : gameObject->components)
		{
			comp->OnRotationUpdate(Maths::Quat(global).Normalize());
		}
	}

	const Maths::Vec3& Transform::GetScale() const
	{
		return scale;
	}

	void Transform::SetScale(const Maths::Vec3& sc)
	{
		scale = sc;
		Update();
		for (auto& comp : gameObject->components)
		{
			comp->OnScaleUpdate(global.GetScaleFromTranslation());
		}
	}
}