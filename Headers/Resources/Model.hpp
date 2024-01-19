#pragma once

#include <vector>

#include "Resources/Mesh.hpp"
#include "Resources/Material.hpp"
#include "Resources/ShaderProgram.hpp"

#include "IResource.hpp"

namespace Resources
{
	class NAT_API Model : public IResource
	{
	public:
		Model();
		~Model() override = default;

		void DeleteData() override;
		void Write(Core::Serialization::Serializer& sr) override;
		void Load(Core::Serialization::Deserializer& dr) override;
		bool ShowEditWindow() override;
		void WindowCreateResource(bool& open) override;
		ObjectType GetType() override { return ObjectType::ModelType; }

		ShaderProgram* shader = nullptr;
		std::vector<Mesh*> meshes;
		std::vector<Material*> materials;
	};
}