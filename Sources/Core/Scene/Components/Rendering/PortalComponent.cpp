#include "Core/Scene/Components/Rendering/PortalComponent.hpp"

#include "Wrappers/Interfacing.hpp"

using namespace Core::Scene::Components;
using namespace Core::Scene::Components::Rendering;

IComponent* PortalComponent::CreateCopy()
{
	return new PortalComponent(*this);
}

void PortalComponent::RenderGui()
{
	PortalBaseComponent::RenderGui();
	interfaceGui->DragFloat3("Target Position", &targetPosition.x, 0.1f);
	interfaceGui->DragQuat("Target Rotation", targetRotation, 0.1f);
}

ComponentType PortalComponent::GetType()
{
	return ComponentType::PortalComponent;
}

void PortalComponent::Serialize(Core::Serialization::Serializer& sr) const
{
	PortalBaseComponent::Serialize(sr);
	sr.Write(targetPosition);
	sr.Write(targetRotation);
}

void PortalComponent::Deserialize(Core::Serialization::Deserializer& dr)
{
	PortalBaseComponent::Deserialize(dr);
	dr.Read(targetPosition);
	dr.Read(targetRotation);
}

LowRenderer::Rendering::Camera PortalComponent::UpdateCamera(const LowRenderer::Rendering::Camera& camera, Maths::Mat4& m, Maths::Vec4& nearPlane) const
{
	LowRenderer::Rendering::Camera result;
	result.fov = camera.fov;
	result.nearPlane = camera.nearPlane;
	result.farPlane = camera.farPlane;
	Maths::Mat4 view = camera.GetViewMatrix();
	Maths::Mat4 half = view * gameObject->transform.GetGlobal();
	Maths::Vec3 normal = Maths::Quat(half) * Maths::Vec3(0, 0, 1);
	nearPlane = Maths::Vec4(normal.UnitVector(), -half.GetPositionFromTranslation().DotProduct(normal));
	//nearPlane.w /= 2.0f;
	Maths::Mat4 target = Maths::Mat4::CreateTransformMatrix(targetPosition, targetRotation);
	Maths::Mat4 inverse = gameObject->transform.GetGlobal().CreateInverseMatrix();
	Maths::Mat4 matrix = target * inverse * view.CreateInverseMatrix();
	result.up = Maths::Mat3(matrix) * result.up;
	result.position = matrix.GetPositionFromTranslation();
	result.focus = -(Maths::Mat3(matrix) * result.focus);
	result.focus += result.position;
	result.Update(camera.GetResolution());
	m = target.CreateInverseMatrix() * gameObject->transform.GetGlobal() * m;
	return result;
}

Maths::Mat4 PortalComponent::UpdateDebugBox() const
{
	return Maths::Mat4::CreateTransformMatrix(targetPosition + boxOffset, targetRotation, boxSize);
}