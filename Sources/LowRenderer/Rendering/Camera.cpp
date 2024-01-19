#include "LowRenderer/Rendering/Camera.hpp"

using namespace Maths;
using namespace LowRenderer::Rendering;

Mat4 Camera::GetViewMatrix() const
{
    return Mat4::CreateViewMatrix(position, focus, deltaUp);
}

Mat4 Camera::GetTransformMatrix() const
{
    return GetViewMatrix().CreateInverseMatrix();
}

Mat4 Camera::GetProjectionMatrix() const
{
    return Mat4::CreatePerspectiveProjectionMatrix(nearPlane, farPlane, fov, aspect_ratio);
}

Mat4 Camera::GetOrthoMatrix() const
{
    return Mat4::CreateOrthoProjectionMatrix(nearPlane, farPlane, fov, aspect_ratio);
}

Maths::Mat4 Camera::GetObliqueProjection(const Maths::Vec4& nearPlane) const
{
    return Mat4::CreateObliqueProjectionMatrix(GetProjectionMatrix(), nearPlane);
}

Maths::Frustum Camera::CreateFrustumFromCamera()
{
    Frustum result;
    f32 halfVSide = farPlane * tanf(Util::ToRadians(fov / 2.0f));
    Vec3 z = (position - focus).UnitVector();
    Vec3 x = up.CrossProduct(z).UnitVector();
    Vec3 y = z.CrossProduct(x);
    Vec3 frontMultFar = z * farPlane;

    result.front = Maths::Vec4(-z, -z.DotProduct(position + z * nearPlane));
    result.back = Maths::Vec4(z, -z.DotProduct(position + frontMultFar));

    Maths::Vec3 n = ((-frontMultFar - x * halfVSide * aspect_ratio).CrossProduct(y)).UnitVector();
    //n = Vec3(n.x, -n.y, -n.z);
    result.right = Maths::Vec4(n, n.DotProduct(position));
    n = (y.CrossProduct(-frontMultFar + x * halfVSide * aspect_ratio)).UnitVector();
    //n = Vec3(n.x, -n.y, -n.z);
    result.left = Maths::Vec4(n, n.DotProduct(position));

    n = (x.CrossProduct(-frontMultFar - y * halfVSide)).UnitVector();
    //n = Vec3(-n.x, n.y, -n.z);
    result.top = Maths::Vec4(n, n.DotProduct(position));
    n = ((-frontMultFar + y * halfVSide).CrossProduct(x)).UnitVector();
    //n = Vec3(-n.x, n.y, -n.z);
    result.bottom = Maths::Vec4(n, n.DotProduct(position));

    return result;
}

void Camera::Update(IVec2 resolution, Vec3 pos, Vec3 forward, Vec3 upIn)
{
    aspect_ratio = resolution.x * 1.0f / resolution.y;
    Resolution = resolution;
    position = pos;
    focus = position + forward;
    deltaUp = upIn;
}

void Camera::Update(IVec2 resolution, Vec3 pos, Vec3 foc)
{
    position = pos;
    focus = foc;
    Update(resolution);
}

void LowRenderer::Rendering::Camera::Update(Maths::IVec2 resolution)
{
    aspect_ratio = resolution.x * 1.0f / resolution.y;
    Resolution = resolution;
    Vec3 forward = (focus - position).UnitVector();
    Vec3 tmpRight = up.CrossProduct(forward);
    if (tmpRight.LengthSquared() < 0.001f) tmpRight = Vec3(1, 0, 0);
    else tmpRight = tmpRight.UnitVector();
    deltaUp = forward.CrossProduct(tmpRight);
}

void Camera::FillMatrixes(Mat4 matrixes[6])
{
    Mat4 proj = GetProjectionMatrix();
    up = Vec3(0, -1, 0);
    for (char i = 0; i < 6; i++)
    {
        Vec3 axis;
        axis[i / 2] = (i % 2) == 0 ? 1.0f : -1.0f;
        focus = position + axis;
        Vec3 tmpRight = up.CrossProduct(axis);
        if (tmpRight.LengthSquared() < 0.001f) tmpRight = Vec3(-1, 0, 0);
        else tmpRight = tmpRight.UnitVector();
        deltaUp = axis.CrossProduct(tmpRight);
        matrixes[static_cast<u32>(i)] = proj * GetViewMatrix();
    }
}