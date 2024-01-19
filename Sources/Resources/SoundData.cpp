#include "Resources/SoundData.hpp"

#include "Resources/ResourceManager.hpp"

using namespace Resources;

void SoundData::DeleteData()
{
}

void SoundData::Write(Core::Serialization::Serializer& sr)
{
	sr.Write(static_cast<u8>(ObjectType::SoundDataType));
	IResource::Write(sr);
	sr.Write(soundData);
}

void SoundData::Load(Core::Serialization::Deserializer& dr)
{
	IResource::Load(dr);
	dr.Read(soundData);
	isLoaded = !soundData.empty();
	isSaved = true;
}
