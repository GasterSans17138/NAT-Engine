#include "Resources/Material.hpp"
#include "Renderer/VulkanRenderer.hpp"
#include "Resources/ResourceManager.hpp"
#include "Resources/StaticTexture.hpp"
#include "Core/App.hpp"

using namespace Resources;
using namespace Core::Serialization;

Material* Material::defaultMaterial = nullptr;

Resources::Material::Material()
{
	albedo = StaticTexture::GetDefaultTexture();
	normal = StaticTexture::GetDefaultNormal();
	height = StaticTexture::GetDefaultTexture();
	sampler = TextureSampler::GetDefaultSampler();
	canBeEdited = true;
}

Resources::Material::~Material()
{
}

void Resources::Material::DeleteData()
{
	DeleteMaterialData();

}

void Resources::Material::DeleteMaterialData()
{
	auto& res = app->GetResources();
	res.Release(albedo);
	res.Release(normal);
	res.Release(height);
	res.Release(sampler);
}

void Resources::Material::Write(Serializer& sr)
{
	sr.Write(static_cast<u8>(ObjectType::MaterialType));
	IResource::Write(sr);
	sr.Write(sampler->hash);
	sr.Write(albedo->hash);
	sr.Write(normal->hash);
	sr.Write(height->hash);
	sr.Write(ambientColor);
	sr.Write(shininess);
}

void Resources::Material::Load(Deserializer& dr)
{
	IResource::Load(dr);
	u64 hash;
	dr.Read(hash);
	sampler = app->GetResources().Get<TextureSampler>(hash);
	if (!sampler) sampler = TextureSampler::GetDefaultSampler();
	dr.Read(hash);
	albedo = app->GetResources().Get<Texture>(hash);
	if (!albedo) albedo = StaticTexture::GetDefaultTexture();
	dr.Read(hash);
	normal = app->GetResources().Get<Texture>(hash);
	if (!normal) normal = StaticTexture::GetDefaultNormal();
	dr.Read(hash);
	height = app->GetResources().Get<Texture>(hash);
	if (!height) height = StaticTexture::GetDefaultTexture();
	dr.Read(ambientColor);
	dr.Read(shininess);
}

bool Resources::Material::ShowEditWindow()
{
	auto& gui = app->GetInterfacing();
	bool open = IResource::BeginEditWindow(gui);
	//ColorEdit3("Material Color", openEditMaterialWindow->ambientColor);
	gui.ColorEdit3("Material Color", ambientColor);
	//SliderFloat("shininess", &openEditMaterialWindow->shininess, 0.01f, 65536.0f, true);
	gui.SliderFloat("shininess", &shininess, 0.01f, 65536.0f, true);
	//Image(openEditMaterialWindow->albedo, Maths::Vec2(20, 20));
	gui.Image(albedo, Maths::Vec2(20, 20));
	gui.SameLine();
	if (gui.Button("Albedo Texture "))
	{
		//selectedTexture = &openEditMaterialWindow->albedo;
		gui.SelectTexture(&albedo);
	}

	gui.SameLine();
	gui.Text(albedo->path.c_str());

	gui.Image(normal, Maths::Vec2(20, 20));
	gui.SameLine();
	if (gui.Button("Normal Texture "))
	{
		//selectedTexture = &normal;
		gui.SelectTexture(&normal);
	}

	gui.SameLine();
	gui.Text(normal->path.c_str());

	gui.Image(height, Maths::Vec2(20, 20));
	gui.SameLine();
	if (gui.Button("Height Texture "))	
	{
		//selectedTexture = &height;
		gui.SelectTexture(&height);
	}

	gui.SameLine();
	gui.Text(height->path.c_str());

	return IResource::EndEditWindow(gui) || !open;
}

Material* Resources::Material::GetDefaultMaterial()
{
	return defaultMaterial;
}

void Resources::Material::SetDefaultMaterial()
{
	defaultMaterial = this;
}

void Material::WindowCreateResource(bool& open)
{
	if (!prevbool && open)
	{
		newMaterial = app->GetResources().CreateResource<Material>("newMaterial");
		SetBasicMaterial(newMaterial);
	}
	prevbool = open;

	if (open)
	{
		Wrappers::Interfacing* gui = &app->GetInterfacing();
		gui->OpenPopup("Create Material");
		if (gui->BeginPopupModal("Create Material"))
		{
			static std::string tempName;
			gui->InputText("name", tempName);

			gui->ColorEdit3("Material Color", newMaterial->ambientColor);
			gui->SliderFloat("shininess", &shininess, 0.01f, 65536.0f, true);
			gui->Image(newMaterial->albedo, Maths::Vec2(20, 20));
			gui->SameLine();
			gui->Text("Albedo Texture ");
			gui->TextureListCombo(&newMaterial->albedo);
			
			gui->SameLine();
			gui->Text(newMaterial->albedo->path.c_str());

			gui->Image(newMaterial->normal, Maths::Vec2(20, 20));
			gui->SameLine();
			gui->Text("Normal Texture ");

			gui->TextureListCombo(&newMaterial->normal);
			gui->SameLine();
			gui->Text(newMaterial->normal->path.c_str());

			gui->Image(newMaterial->height, Maths::Vec2(20, 20));
			gui->SameLine();
			gui->Text("Height Texture ");
			gui->TextureListCombo(&newMaterial->height);
			gui->SameLine();
			gui->Text(newMaterial->height->path.c_str());


			if (gui->Button("create"))
			{
				newMaterial->path = tempName;
				if (newMaterial->path.size() > 0)
				{
					gui->CloseCurrentPopup();
					open = false;
					tempName = "";
				}
				else
				{
					DEBUG_LOG("your resource need name\n");
				}
			}


			if (gui->Button("close"))
			{
				if (!newMaterial->isLoaded)
					app->GetResources().GetMaterialList().pop_back();
				gui->CloseCurrentPopup();
				open = false;
			}
			gui->EndPopup();
		}
	}

	

}
void Material::SetBasicMaterial(Material* mat)
{
	mat->albedo = app->GetResources().GetTextureList()[0];
	mat->normal = app->GetResources().GetTextureList()[0];
	mat->height = app->GetResources().GetTextureList()[0];
}