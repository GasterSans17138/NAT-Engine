#include "Resources/Mesh.hpp"
#include "Resources/ResourceManager.hpp"
#include "Renderer/VulkanRenderer.hpp"
#include "Core/App.hpp"
using namespace Resources;
using namespace Core::Serialization;
Mesh::~Mesh()
{
}

void Mesh::UpdateVectors()
{
	verticeCount = static_cast<u32>(vertices.size());
	indiceCount = static_cast<u32>(indices.size());
}

u32 Mesh::GetVertexCount() const 
{
	return verticeCount;
}

u32 Mesh::GetIndexCount() const
{
	return indiceCount;
}

void Resources::Mesh::DeleteData()
{
	vertices.clear();
	indices.clear();

	vertices.shrink_to_fit();
	indices.shrink_to_fit();

	app->GetRenderer().UnloadMesh(this);
}

void Mesh::Write(Serializer& sr)
{
	sr.Write(static_cast<u8>(ObjectType::MeshType));
	IResource::Write(sr);
	sr.Write(aabb);
	sr.Write(indices.size());
	for (auto& index : indices)
	{
		sr.Write(index);
	}
	sr.Write(vertices.size());
	for (auto& vertex : vertices)
	{
		sr.Write(vertex.pos);
		sr.Write(vertex.color);
		sr.Write(vertex.uv);
		sr.Write(vertex.normal);
		sr.Write(vertex.tangent);
	}
}

void Mesh::Load(Deserializer& dr)
{
	IResource::Load(dr);
	dr.Read(aabb);
	u64 size;
	dr.Read(size);
	indices.resize(size);
	for (auto& index : indices)
	{
		dr.Read(index);
	}
	dr.Read(size);
	vertices.resize(size);
	for (auto& vertex : vertices)
	{
		dr.Read(vertex.pos);
		dr.Read(vertex.color);
		dr.Read(vertex.uv);
		dr.Read(vertex.normal);
		dr.Read(vertex.tangent);
	}
	UpdateVectors();
	isLoaded = app->GetRenderer().LoadMesh(this);
}

void Mesh::WindowCreateResource(bool& open)
{
	
}