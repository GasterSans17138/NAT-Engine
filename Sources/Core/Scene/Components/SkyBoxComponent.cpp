#include "Core/App.hpp"

#include "Resources/ResourceManager.hpp"
#include "Resources/StaticCubeMap.hpp"
#include "Resources/StaticTexture.hpp"
#include "Core/Scene/Components/SkyBoxComponent.hpp"

using namespace Core::Scene::Components;

Resources::Mesh* SkyBoxComponent::boxMesh = nullptr;

SkyBoxComponent::SkyBoxComponent() : IComponent()
{
}

void SkyBoxComponent::Render(const Maths::Mat4& mvp, const Maths::Mat4& vp, const Maths::Mat4& modelOverride, const Maths::Frustum& cameraFrustum, LowRenderer::RenderPassType pass)
{
	if (!boxMesh)
	{
		boxMesh = App::GetInstance()->GetResources().Get<Resources::Mesh>(0x31);
	}
	if (!cubeMap || !skyShader) return;
	auto& renderer = App::GetInstance()->GetRenderer();
	renderer.BindShader(skyShader);
	auto mainCam = SceneManager::GetInstance()->currentCamera;
	Maths::Mat4 matrix = mainCam->GetViewMatrix();
	Maths::Vec4 pos = Maths::Vec4(0,0,0,1);
	for (u8 i = 0; i < 4; i++)
	{
		matrix.at(3, i) = pos[i];
	}
	matrix = mainCam->GetProjectionMatrix() * matrix;
	if (pass & LowRenderer::RenderPassType::OBJECT)
	{
		renderer.RenderMeshObject(boxMesh, matrix, SceneManager::GetInstance()->GetNextIndex(gameObject));
	}
	else
	{
		std::vector<const Renderer::RendererTexture*> textures;
		textures.push_back(&cubeMap->GetRendererTexture());
		textures.push_back(&Resources::StaticTexture::GetDefaultNormal()->GetRendererTexture());
		textures.push_back(&Resources::StaticTexture::GetDefaultTexture()->GetRendererTexture());
		Resources::Material mat;
		*reinterpret_cast<f64*>(&mat.ambientColor.x) = App::GetInstance()->GetWindow().GetWindowTime();
		renderer.RenderMesh(boxMesh, modelOverride * gameObject->transform.GetGlobal(), matrix, textures, &mat);
	}
}

IComponent* SkyBoxComponent::CreateCopy()
{
	return new SkyBoxComponent(*this);
}

void SkyBoxComponent::RenderGui()
{
	interfaceGui->ShaderCombo(Resources::ShaderVariant::SkyBox, &skyShader);
	interfaceGui->Text(cubeMap ? cubeMap->path.c_str() : "missingno");
	bool clicked = interfaceGui->IsClicked();
	interfaceGui->Image(cubeMap ? cubeMap->GetImGuiRendererTexture() : Resources::StaticTexture::GetDebugTexture()->GetRendererTexture(), Maths::IVec2(20, 20));
	if (interfaceGui->IsClicked() || clicked)
	{
		interfaceGui->SelectCubeMap(&cubeMap);
	}
}

ComponentType Core::Scene::Components::SkyBoxComponent::GetType()
{
	return ComponentType::SkyBox;
}

void Core::Scene::Components::SkyBoxComponent::Serialize(Core::Serialization::Serializer& sr) const
{
	sr.Write(cubeMap ? cubeMap->hash : (u64)0);
	sr.Write(skyShader ? skyShader->hash : (u64)0);
}

void Core::Scene::Components::SkyBoxComponent::Deserialize(Core::Serialization::Deserializer& dr)
{
	u64 hash;
	dr.Read(hash);
	auto& res = App::GetInstance()->GetResources();
	cubeMap = res.Get<Resources::CubeMap>(hash);
	dr.Read(hash);
	skyShader = res.Get<Resources::ShaderProgram>(hash);
}

void SkyBoxComponent::Delete()
{
	auto& res = App::GetInstance()->GetResources();
	res.Release(cubeMap);
	res.Release(skyShader);
}
