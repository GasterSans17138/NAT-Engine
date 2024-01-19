#pragma once

#include "Camera.hpp"

namespace Wrappers
{
	class WindowManager;
}

namespace LowRenderer::Rendering
{
	class NAT_API FlyingCamera : public Camera
	{
	public:
		FlyingCamera() = default;
		~FlyingCamera() = default;

		Maths::Vec3 VirtualPos = focus;
		Maths::Vec3 Rotation;
		f32 Distance = 1.0f;
		f32 MovementSpeed = 25.0f;
		f32 RotationSpeed = 0.12f;

		using Camera::Update;
		void Update(Maths::IVec2 resolution, Wrappers::WindowManager* window);

	protected:
	};
}