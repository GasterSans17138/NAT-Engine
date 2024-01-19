#pragma once

#include "Maths/Maths.hpp"

namespace LowRenderer::Rendering
{
	class NAT_API Camera
	{
	public:
		Camera() = default;
		~Camera() = default;

		Maths::Vec3 position;
		Maths::Vec3 focus = Maths::Vec3(0, 0, 1);
		Maths::Vec3 up = Maths::Vec3(0, 1, 0);
		f32 fov = 60;
		f32 nearPlane = 0.1f;
		f32 farPlane = 10000.0f;

		void Update(Maths::IVec2 resolution, Maths::Vec3 position, Maths::Vec3 forward, Maths::Vec3 up);
		void Update(Maths::IVec2 resolution, Maths::Vec3 pos, Maths::Vec3 foc);
		void Update(Maths::IVec2 resolution);

		Maths::Mat4 GetViewMatrix() const;
		Maths::Mat4 GetTransformMatrix() const;
		Maths::Mat4 GetProjectionMatrix() const;
		Maths::Mat4 GetOrthoMatrix() const;
		Maths::Mat4 GetObliqueProjection(const Maths::Vec4& nearPlane) const;
		Maths::Frustum CreateFrustumFromCamera();

		Maths::IVec2 GetResolution() const { return Resolution; }
		Maths::Vec3 GetDeltaUp() const { return deltaUp; }
		void FillMatrixes(Maths::Mat4 matrixes[6]);

	protected:
		Maths::IVec2 Resolution;
		Maths::Vec3 deltaUp;
		f32 aspect_ratio = 1.0f;
	};
}