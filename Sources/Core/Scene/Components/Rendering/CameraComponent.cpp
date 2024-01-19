#include "Core/Scene/Components/Rendering/CameraComponent.hpp"

#include "Core/Scene/GameObject.hpp"
#include "Core/App.hpp"

using namespace Core::Scene::Components::Rendering;
using namespace Core::Scene::Components;

Core::Scene::SceneManager* CameraComponent::scenes = nullptr;
Resources::ShaderProgram* CameraComponent::boxShader = nullptr;

CameraComponent::CameraComponent() : IComponent()
{
	interfaceGui = &Core::App::GetInstance()->GetInterfacing();
}

CameraComponent::CameraComponent(const CameraComponent& other) : IComponent()
{
	interfaceGui = &Core::App::GetInstance()->GetInterfacing();
	frameBuffer = other.frameBuffer;
	camera = other.camera;
}

IComponent* CameraComponent::CreateCopy()
{
	return new CameraComponent(*this);
}

void CameraComponent::DataUpdate()
{
	if (!scenes)
	{
		scenes = &Core::App::GetInstance()->GetSceneManager();
		boxShader = Core::App::GetInstance()->GetResources().Get<Resources::ShaderProgram>(0x2b);
	}
	if (!frameBuffer)
	{
		frameBuffer = Core::App::GetInstance()->GetResources().CreateResource<LowRenderer::FrameBuffer>("Render buffer");
		frameBuffer->CreateFrameBuffer();
		tmpResolution = frameBuffer->GetResolution();
		frameBuffer->isLoaded = true;
	}
	frameBuffer->Update();
	Maths::Mat3 rot(gameObject->transform.GetGlobal());
	camera.Update(frameBuffer->GetResolution(), gameObject->transform.GetGlobal().GetPositionFromTranslation(), rot * Maths::Vec3(0, 0, 1), rot * Maths::Vec3(0, 1, 0));
	scenes->PushRenderCamera(this);
}

void CameraComponent::Render(const Maths::Mat4& mvp, const Maths::Mat4& vp, const Maths::Mat4& modelOverride, const Maths::Frustum& cameraFrustum, LowRenderer::RenderPassType pass)
{
	if (pass == LowRenderer::RenderPassType::DEFAULT && interfaceGui->GetSelectedGameObject() == gameObject)
	{
		Maths::Mat4 cam = (camera.GetProjectionMatrix() * camera.GetViewMatrix()).CreateInverseMatrix();
		Maths::Mat4 m = vp * cam;
		App::GetInstance()->GetRenderer().BindShader(boxShader);
		App::GetInstance()->GetRenderer().DrawVertices(36, m, m);
	}
}

Maths::Mat4 CameraComponent::GetVPMatrix() const
{
	if (!gameObject)
	{
		LOG(DEBUG_LEVEL::LWARNING, "Camera with no GameObject !");
		return Maths::Mat4(1);
	}
	return camera.GetProjectionMatrix() * camera.GetViewMatrix();
}

void CameraComponent::RenderGui()
{
	interfaceGui->Image(frameBuffer, Maths::Vec2(64,64));
	interfaceGui->DragFloat("Camera FOV", &camera.fov);
	interfaceGui->DragFloat("Near Plane", &camera.nearPlane);
	interfaceGui->DragFloat("Far Plane", &camera.farPlane);
	Maths::Vec4 clearColor = frameBuffer->ClearColor;
	interfaceGui->ColorEdit4("Clear Color", clearColor);
	if (clearColor != frameBuffer->ClearColor)
	{
		frameBuffer->ClearColor = clearColor;
		frameBuffer->UpdateClearColor();
	}
	interfaceGui->DragInt2("Framebuffer Resolution", &tmpResolution.x);
	if (tmpResolution != frameBuffer->GetResolution() && interfaceGui->Button("Valid Changes"))
	{
		frameBuffer->Resize(tmpResolution);
	}
}

ComponentType CameraComponent::GetType()
{
	return ComponentType::CameraComponent;
}

void CameraComponent::Serialize(Core::Serialization::Serializer& sr) const
{
	sr.Write(camera.nearPlane);
	sr.Write(camera.farPlane);
	sr.Write(camera.fov);
	if (frameBuffer) sr.Write(frameBuffer->hash);
}

void CameraComponent::Deserialize(Core::Serialization::Deserializer& dr)
{
	dr.Read(camera.nearPlane);
	dr.Read(camera.farPlane);
	dr.Read(camera.fov);
	u64 hash;
	if (dr.Read(hash))
	{
		frameBuffer = App::GetInstance()->GetResources().Get<LowRenderer::FrameBuffer>(hash);
		if (frameBuffer) tmpResolution = frameBuffer->GetResolution();
	}
}

void CameraComponent::Delete()
{
	App::GetInstance()->GetResources().Release(frameBuffer);
}
