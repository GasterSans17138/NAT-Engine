#include "Resources/Model.hpp"

#include "Core/Serialization/Serializer.hpp"
#include "Core/FileManager.hpp"
#include "Resources/ResourceManager.hpp"
#include "Resources/Texture.hpp"
#include "Renderer/VulkanRenderer.hpp"
#include "Core/App.hpp"

using namespace Core::Serialization;
using namespace Resources;
using namespace Core::FileManager;

void Model::Write(Serializer& sr)
{
	sr.Write(static_cast<u8>(ObjectType::ModelType));
	IResource::Write(sr);
	sr.Write(meshes.size());
	for (auto& mesh : meshes)
	{
		sr.Write(mesh->hash);
	}
	sr.Write(materials.size());
	for (auto& material : materials)
	{
		sr.Write(material->hash);
	}
}

bool Model::ShowEditWindow()
{
	auto& gui = app->GetInterfacing();
	bool open = IResource::BeginEditWindow(gui);
	gui.Text("mesh :");
	for (u64 i = 0; i < meshes.size(); i++)
	{
		gui.PushId((s32)i);
		bool visible = true;
		bool up = gui.ArrowButton("Up", 2);
		gui.SameLine();
		bool down = gui.ArrowButton("Down", 3);
		gui.SameLine();
		gui.SetCursorX(60);
		bool open = gui.Button(meshes[i]->path.c_str());
		if (!visible)
		{
			delete meshes[i];
			for (u64 j = i; j < meshes.size() - 1; j++)
			{
				meshes[j] = meshes[j + 1];
			}
			meshes.pop_back();
		}
		else
		{
			if (open) gui.SelectMesh(&meshes[i]);
			if (up && i)
			{
				std::swap(meshes[i], meshes[i - 1]);
			}
			else if (down && i < meshes.size() - 1)
			{
				std::swap(meshes[i], meshes[i + 1]);
			}
		}
		gui.PopId();
	}
	gui.Separator();
	for (u64 i = 0; i < materials.size(); i++)
	{
		gui.Text("material :");
		gui.SameLine();
		if (gui.Button(materials[i] ? materials[i]->path.c_str() : "missingno"))
		{
			gui.SelectMaterial(&materials[i]);
		}
	}
	return IResource::EndEditWindow(gui) || !open;
}

Resources::Model::Model()
{
	shader = ShaderProgram::GetDefaultShader();
	canBeEdited = true;
}

void Model::DeleteData()
{
	app->GetRenderer().UnLoadModel(this);
	auto& res = app->GetResources();
	for (auto& mat : materials)
	{
		res.Release(mat);
	}
}

void Model::Load(Deserializer& dr)
{
	IResource::Load(dr);
	u64 size;
	dr.Read(size);
	meshes.resize(size);
	for (auto& mesh : meshes)
	{
		u64 hash;
		dr.Read(hash);
		mesh = app->GetResources().Get<Resources::Mesh>((hash));
	}
	dr.Read(size);
	materials.resize(size);
	for (auto& material : materials)
	{
		u64 hash;
		dr.Read(hash);
		material = app->GetResources().Get<Resources::Material>((hash));
	}
}

void Model::WindowCreateResource(bool& open)
{
	
}