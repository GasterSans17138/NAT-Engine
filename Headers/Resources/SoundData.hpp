#pragma once

#include <string>

#include "IResource.hpp"
#include "ResourceManager.hpp"

namespace Resources
{
	class NAT_API SoundData : public IResource
	{
	public:
		SoundData() = default;

		~SoundData() override = default;

		void DeleteData() override;
		void Write(Core::Serialization::Serializer& sr) override;
		void Load(Core::Serialization::Deserializer& dr) override;
		ObjectType GetType() override { return ObjectType::SoundDataType; }

		std::string soundData;
	private:
	};

}