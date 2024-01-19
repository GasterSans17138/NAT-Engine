#include "Core/Scene/Components/Rendering/PortalBaseComponent.hpp"

#include "Core/App.hpp"
#include "Core/Scene/SceneManager.hpp"

using namespace Core::Scene::Components::Rendering;
using namespace Core::Scene::Components;

Core::Scene::SceneManager* PortalBaseComponent::scenes = nullptr;
Resources::ShaderProgram* PortalBaseComponent::portalShader = nullptr;
Resources::ShaderProgram* PortalBaseComponent::depthShader = nullptr;
Resources::ShaderProgram* PortalBaseComponent::boxShader = nullptr;
Resources::Mesh* PortalBaseComponent::boxMesh = nullptr;

void PortalBaseComponent::Render(const Maths::Mat4& mvp, const Maths::Mat4& vp, const Maths::Mat4& modelOverride, const Maths::Frustum& cameraFrustum, LowRenderer::RenderPassType pass)
{
	if (!(pass & (LowRenderer::OBJECT | LowRenderer::DEFAULT)) || !portalShader) return;
	auto& renderer = App::GetInstance()->GetRenderer();
	if (pass & LowRenderer::DEFAULT && interfaceGui->GetSelectedGameObject() == gameObject)
	{
		Maths::Mat4 m = mvp * (Maths::Mat4::CreateTranslationMatrix(boxOffset) * Maths::Mat4::CreateScaleMatrix(boxSize));
		renderer.BindShader(boxShader);
		renderer.RenderMesh(boxMesh, m, m);
	}
	else if (pass & LowRenderer::OBJECT && portalMesh)
	{
		renderer.BindShader(portalShader);
		renderer.RenderMeshObject(portalMesh, mvp, SceneManager::GetInstance()->GetNextIndex(gameObject));
	}
}

void PortalBaseComponent::Overwrite(const Maths::Mat4& mvp, Renderer::StencilState state, u8 value, const Maths::Vec3& color)
{
	if (!portalShader || !portalMesh) return;
	auto& renderer = App::GetInstance()->GetRenderer();
	renderer.SetStencilState(value, state);
	renderer.BindShader((state == Renderer::StencilState::NO_DEPTH || state == Renderer::StencilState::GREATER) ? depthShader : portalShader);
	renderer.RenderMesh(portalMesh, mvp, mvp, color);
}

void PortalBaseComponent::DataUpdate()
{
	if (!scenes)
	{
		auto instance = Core::App::GetInstance();
		scenes = &instance->GetSceneManager();
		portalShader = instance->GetResources().Get<Resources::ShaderProgram>(0x2e);
		depthShader = instance->GetResources().Get<Resources::ShaderProgram>(0x30);
		boxShader = instance->GetResources().Get<Resources::ShaderProgram>(0x29);
		boxMesh = instance->GetResources().Get<Resources::Mesh>(0x31);
	}
	scenes->PushPortalObject(this);
}

void PortalBaseComponent::RenderGui()
{
	interfaceGui->Text("Mesh     :");
	interfaceGui->SameLine();
	if (interfaceGui->Button(portalMesh ? portalMesh->path.c_str() : "(missingno)"))
	{
		interfaceGui->SelectMesh(&portalMesh);
	}
	interfaceGui->DragFloat3("Cull Box Size", &boxSize.x, 0.1f);
	interfaceGui->DragFloat3("Cull Box Offset", &boxOffset.x, 0.1f);
}

void PortalBaseComponent::Serialize(Core::Serialization::Serializer& sr) const
{
	u64 hash = 0;
	if (portalMesh) hash = portalMesh->hash;
	sr.Write(hash);
	sr.Write(boxSize);
	sr.Write(boxOffset);
}

void PortalBaseComponent::Deserialize(Core::Serialization::Deserializer& dr)
{
	u64 hash = 0;
	dr.Read(hash);
	if (hash) portalMesh = Core::App::GetInstance()->GetResources().Get<Resources::Mesh>(hash);
	dr.Read(boxSize);
	dr.Read(boxOffset);
}

bool PortalBaseComponent::IsVisibleOnScreen(const LowRenderer::Rendering::Camera* const targetCam)
{
	if (!boxMesh || !portalMesh || (gameObject->transform.GetGlobal().GetPositionFromTranslation() - targetCam->position).DotProduct((gameObject->transform.GetGlobal() * Maths::Vec4(0,0,-1,0)).GetVector()) < 0) return false;
	for (auto& vert : boxMesh->vertices)
	{
		Maths::Vec3 pos = (gameObject->transform.GetGlobal() * Maths::Vec4(vert.pos * boxSize + boxOffset)).GetVector();
		if ((pos - targetCam->position).DotProduct(targetCam->focus - targetCam->position) > 0.0f) return true;
	}
	return false;
}

Maths::Vec4 PortalBaseComponent::GetScreenCovering(const Maths::Mat4& mvp)
{
	Maths::Vec4 result = Maths::Vec4(1.0f,1.0f,-1.0f,-1.0f);
	for (auto& vert : boxMesh->vertices)
	{
		Maths::Vec4 vec = (mvp * Maths::Vec4(vert.pos * boxSize + boxOffset));
		if (vec.w <= 0.0f) continue;
		Maths::Vec3 pos = vec.Homogenize().GetVector();
		result = Maths::Vec4(Maths::Util::MinF(result.x, pos.x), Maths::Util::MinF(result.y, pos.y), Maths::Util::MaxF(result.z, pos.x), Maths::Util::MaxF(result.w, pos.y));
	}
	return result;
}

void PortalBaseComponent::Delete()
{
	App::GetInstance()->GetResources().Release(portalMesh);
}