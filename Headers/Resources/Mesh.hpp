#pragma once

#include <vector>

#include "Core/Types.hpp"

#include "Renderer/RendererVertex.hpp"
#include "Renderer/RendererMesh.hpp"
#include "ResourceManager.hpp"
#include "Core/Types.hpp"
#include "IResource.hpp"

namespace Resources
{
	class NAT_API Mesh : public IResource
	{
		//
		//Mesh
		//

	public:
		Mesh() = default;
		~Mesh() override;

		u32 GetVertexCount() const;
		u32 GetIndexCount() const;
		ObjectType GetType() override { return ObjectType::MeshType; }

		void UpdateVectors();

		std::vector<Renderer::RendererVertex> vertices;
		std::vector<u32> indices;
		
		Renderer::RendererMesh rendererMesh;
		Maths::AABB aabb;

		//
		// IResource
		//

	public:
		void DeleteData() override;
		void Write(Core::Serialization::Serializer& sr) override;
		void Load(Core::Serialization::Deserializer& dr) override;
		void WindowCreateResource(bool& open) override;
	private :
		u32 verticeCount = 0;
		u32 indiceCount = 0;
	};
}