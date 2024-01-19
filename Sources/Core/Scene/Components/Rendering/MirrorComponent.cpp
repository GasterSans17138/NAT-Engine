#include "Core/Scene/Components/Rendering/MirrorComponent.hpp"

#include "Wrappers/Interfacing.hpp"

using namespace Core::Scene::Components;
using namespace Core::Scene::Components::Rendering;

IComponent* MirrorComponent::CreateCopy()
{
	return new MirrorComponent(*this);
}

void MirrorComponent::RenderGui()
{
	PortalBaseComponent::RenderGui();
}

ComponentType MirrorComponent::GetType()
{
	return ComponentType::PortalComponent;
}

void MirrorComponent::Serialize(Core::Serialization::Serializer& sr) const
{
	PortalBaseComponent::Serialize(sr);
}

void MirrorComponent::Deserialize(Core::Serialization::Deserializer& dr)
{
	PortalBaseComponent::Deserialize(dr);
}

LowRenderer::Rendering::Camera MirrorComponent::UpdateCamera(const LowRenderer::Rendering::Camera& camera, Maths::Mat4& m, Maths::Vec4& nearPlane) const
{
	LowRenderer::Rendering::Camera result(camera);
	Maths::Vec3 portalPos = gameObject->transform.GetGlobal().GetPositionFromTranslation();
	Maths::Vec3 normal = Maths::Quat(gameObject->transform.GetGlobal()) * Maths::Vec3(-1,0,0);
	result.position = (result.position - portalPos).Reflect(normal) + portalPos;
	result.up = (result.up - portalPos).Reflect(normal) + portalPos;
	result.focus = (result.focus - portalPos).Reflect(normal) + portalPos;
	result.Update(result.GetResolution());
	m = gameObject->transform.GetGlobal() * m;
	return result;
}

Maths::Mat4 MirrorComponent::UpdateDebugBox() const
{
	return Maths::Mat4::CreateYRotationMatrix(180.0f);
}
