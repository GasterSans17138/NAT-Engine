#include "Core/App.hpp"

#include "Resources/ResourceManager.hpp"
#include "Resources/Mesh.hpp"
#include "Core/Scene/Components/RenderModelComponent.hpp"

using namespace Core::Scene::Components;

RenderModelComponent::RenderModelComponent() : IComponent()
{
}

void RenderModelComponent::UseModel(Resources::Model* pModel)
{
	mUsedModel = pModel;
	meshes.clear();

	for (Resources::Mesh* mesh : pModel->meshes)
	{
		this->meshes.push_back(mesh);
	}

	materials.clear();

	for (Resources::Material* mat : pModel->materials)
	{
		materials.push_back(mat);
	}

	if(pModel->shader != nullptr)
		shader = pModel->shader;
}

void RenderModelComponent::Render(const Maths::Mat4& mvp, const Maths::Mat4& vp, const Maths::Mat4& modelOverride, const Maths::Frustum& cameraFrustum, LowRenderer::RenderPassType pass)
{
	if (materials.size() != meshes.size())
	{
		if (materials.size() > meshes.size())
		{
			materials.resize(meshes.size());
		}
		else if (materials.size() < meshes.size())
		{
			for (u64 i = materials.size(); i < meshes.size(); i++)
			{
				materials.push_back(Resources::Material::GetDefaultMaterial());
			}
		}
	}
	if (!shader) shader = Resources::ShaderProgram::GetDefaultShader();
	if (!meshes.size() || (hideFirst && !(pass & LowRenderer::RenderPassType::SECONDARY)) || (hideSecond && (pass & LowRenderer::RenderPassType::SECONDARY))) return;
	auto& renderer = App::GetInstance()->GetRenderer();
	renderer.BindShader(shader);
	if (pass == LowRenderer::RenderPassType::OBJECT)
	{
		for (auto& mesh : meshes)
		{
			if (!mesh) continue;
			renderer.RenderMeshObject(mesh, mvp, SceneManager::GetInstance()->GetNextIndex(gameObject));
		}
	}
	else if (pass & targetPass)
	{
		for (u64 i = 0; i < meshes.size(); ++i)
		{
			if (!meshes[i] || (useCulling && !meshes[i]->aabb.IsOnFrustum(cameraFrustum, gameObject->transform.GetGlobal()))) continue; // GET CULLED IDIOT
			renderer.RenderMesh(meshes[i], gameObject->transform.GetGlobal(), mvp, materials[i]);
		}
		if (interfaceGui->GetSelectedGameObject() != gameObject) return;
		for (u64 i = 0; i < meshes.size(); ++i)
		{
			if (!meshes[i]) continue;
			static Resources::ShaderProgram* boxShader = App::GetInstance()->GetResources().Get<Resources::ShaderProgram>(0x29);
			static Resources::Mesh* boxMesh = App::GetInstance()->GetResources().Get<Resources::Mesh>(0x31);
			Maths::Mat4 m = mvp * (Maths::Mat4::CreateTranslationMatrix(meshes[i]->aabb.center) * Maths::Mat4::CreateScaleMatrix(meshes[i]->aabb.size));
			renderer.BindShader(boxShader);
			renderer.RenderMesh(boxMesh, m, m, Maths::Vec3(0,1,0));
		}
	}
}

IComponent* RenderModelComponent::CreateCopy()
{
	return new RenderModelComponent(*this);
}

void RenderModelComponent::RenderGui()
{
	if (interfaceGui->ModelListCombo(&mUsedModel))
		this->UseModel(mUsedModel);
	std::vector<Resources::ShaderVariant> filters = { Resources::ShaderVariant::Default , Resources::ShaderVariant::WireFrame };
	
	interfaceGui->Text("Shader :");
	interfaceGui->SameLine();
	interfaceGui->ShaderCombo(filters, &shader);
	interfaceGui->CheckBox("Use Culling", &useCulling);
	interfaceGui->CheckBox("Hide in default pass", &hideFirst);
	interfaceGui->CheckBox("Hide in second pass", &hideSecond);
	interfaceGui->Separator();

	if (HasMeshes() && ImGui::TreeNode("Materials :"))
	{
		for (int i = 0; i < meshes.size(); i++)
		{
			interfaceGui->PushId(i);
			interfaceGui->Text((std::to_string(i) + " : ").c_str());
			interfaceGui->SameLine();
			interfaceGui->MaterialListCombo(&materials[i]);
			interfaceGui->PopId();
		}
		ImGui::TreePop();
	}
	if (HasMeshes() && ImGui::TreeNode("Meshes :"))
	{
		for (u64 i = 0; i < meshes.size(); i++)
		{
			bool up = interfaceGui->ArrowButton("Up", 2);
			interfaceGui->SameLine();
			bool down = interfaceGui->ArrowButton("Down", 3);
			interfaceGui->SameLine();
			if (up && i)
			{
				std::swap(meshes[i], meshes[i - 1]);
			}
			else if (down && i + 1 < meshes.size())
			{
				std::swap(meshes[i], meshes[i + 1]);
			}
			interfaceGui->PopId();
			interfaceGui->PushId(i);
			interfaceGui->Text((std::to_string(i) + " : ").c_str());
			interfaceGui->SameLine();
			interfaceGui->MeshListCombo(&meshes[i]);
		}
		ImGui::TreePop();
	}

	if (interfaceGui->Button("Add"))
	{
		materials.push_back(Resources::Material::GetDefaultMaterial());
		meshes.push_back(nullptr);
	}
	if (meshes.size() && interfaceGui->Button("Delete"))
	{
		materials.pop_back();
		meshes.pop_back();
	}

	interfaceGui->Separator();
}

ComponentType RenderModelComponent::GetType()
{
	return ComponentType::RenderModel;
}

void RenderModelComponent::Serialize(Core::Serialization::Serializer& sr) const
{
	sr.Write(static_cast<u8>(targetPass));
	u8 mask = (u8)useCulling | (u8)hideFirst << 1 | (u8)hideSecond << 2;
	sr.Write(mask);
	sr.Write(shader->hash);
	sr.Write(meshes.size());
	for (auto& mesh : meshes)
	{
		sr.Write(mesh ? mesh->hash : (u64)0);
	}
	sr.Write(materials.size());
	for (auto& mat : materials)
	{
		sr.Write(mat->hash);
	}
}

void RenderModelComponent::Deserialize(Core::Serialization::Deserializer& dr)
{
	dr.Read(reinterpret_cast<u8&>(targetPass));
	u8 mask = 0;
	dr.Read(mask);
	useCulling = mask & 0x1;
	hideFirst = mask & 0x2;
	hideSecond = mask & 0x4;
	u64 hash;
	dr.Read(hash);
	auto& res = App::GetInstance()->GetResources();
	shader = res.Get<Resources::ShaderProgram>(hash);
	if (!shader) shader = Resources::ShaderProgram::GetDefaultShader();
	u64 size;
	dr.Read(size);
	for (u64 i = 0; i < size; i++)
	{
		dr.Read(hash);
		meshes.push_back(nullptr);
		if (hash) meshes.back() = res.Get<Resources::Mesh>(hash);
	}
	dr.Read(size);
	for (u64 i = 0; i < size; i++)
	{
		dr.Read(hash);
		materials.push_back(nullptr);
		if (hash) materials.back() = res.Get<Resources::Material>(hash);
		if (!materials.back()) materials.back() = Resources::Material::GetDefaultMaterial();
	}
}

void RenderModelComponent::Delete()
{
	auto& res = App::GetInstance()->GetResources();
	res.Release(shader);
	for (u64 i = 0; i < meshes.size(); i++)
	{
		res.Release(meshes[i]);
		res.Release(materials[i]);
	}
}
