#include "LowRenderer/Rendering/FlyingCamera.hpp"

#include "Wrappers/WindowManager.hpp"

using namespace LowRenderer::Rendering;
using namespace Maths;

void FlyingCamera::Update(IVec2 resolution, Wrappers::WindowManager* window)
{
    if (window->IsCursorCaptured())
    {
        Distance = Util::Clamp(Distance - window->GetMouseScrollDelta() / 3, 0.0f, 100.0f);
        // Update rotation.
        Rotation = Rotation + Vec3(-window->GetCursorDelta().x, window->GetCursorDelta().y, 0) * RotationSpeed;
        Rotation = Vec3(Util::Mod(Rotation.x, 360), Util::Mod(Rotation.y, 360), Util::Mod(Rotation.z, 360));
        // Update focus.
        float dSpeed = window->GetDeltaTime() * MovementSpeed * (window->IsKeyDown(GLFW_KEY_LEFT_SHIFT) ? 5.0f : (window->IsKeyDown(GLFW_KEY_LEFT_CONTROL) ? 0.2f : 1.0f));
        Vec3 delta = Vec3(dSpeed * window->IsKeyDown(GLFW_KEY_D) - dSpeed * window->IsKeyDown(GLFW_KEY_A), dSpeed * window->IsKeyDown(GLFW_KEY_E) - dSpeed * window->IsKeyDown(GLFW_KEY_Q), dSpeed * window->IsKeyDown(GLFW_KEY_S) - dSpeed * window->IsKeyDown(GLFW_KEY_W));
        delta = Vec3(cosf(Util::ToRadians(Rotation.x)) * delta.x + sinf(Util::ToRadians(Rotation.x)) * delta.z, delta.y, -sinf(Util::ToRadians(Rotation.x)) * delta.x + cosf(Util::ToRadians(Rotation.x)) * delta.z);
        VirtualPos = VirtualPos + delta;
    }
    Mat4 Rot = Mat4::CreateRotationMatrix(Vec3(-Rotation.y, Rotation.x, Rotation.z));
    Vec3 tmp = (Rot * Vec3(0, 0, 1)).GetVector();
    position = VirtualPos + tmp * Distance;
    focus = position + tmp * -1;
    aspect_ratio = resolution.x * 1.0f / resolution.y;
    Resolution = resolution;
    deltaUp = (Rot * up).GetVector();
}