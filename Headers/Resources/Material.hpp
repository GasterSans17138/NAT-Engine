#pragma once

#include "IResource.hpp"
#include "Texture.hpp"
#include "TextureSampler.hpp"

namespace Core
{
	class App;
}

namespace Resources
{
	class ResourceManager;
	class NAT_API Material : public IResource
	{
	public:
		Material();

		~Material() override;

		void DeleteData() override;
		void DeleteMaterialData();
		void Write(Core::Serialization::Serializer& sr) override;
		void Load(Core::Serialization::Deserializer& dr) override;
		bool ShowEditWindow() override;
		void WindowCreateResource(bool& open) override;
		ObjectType GetType() override { return ObjectType::MaterialType; }

		static Material* GetDefaultMaterial();

		Texture* albedo = nullptr;
		Texture* normal = nullptr;
		Texture* height = nullptr;
		Maths::Vec3 ambientColor = Maths::Vec3(1);
		f32 shininess = 256;
		Resources::TextureSampler* sampler;


	private:
		void SetDefaultMaterial();

		static Material* defaultMaterial;

		friend Core::App;
		Material* newMaterial = nullptr;
		bool prevbool = false;
		void SetBasicMaterial(Material* mat);
	};

}