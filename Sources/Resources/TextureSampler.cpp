#include "Resources/TextureSampler.hpp"

#include "Renderer/VulkanRenderer.hpp"
#include "Core/App.hpp"

using namespace Resources;

TextureSampler* TextureSampler::defaultSampler;

TextureSampler::TextureSampler()
{
}

TextureSampler::~TextureSampler()
{
}

void TextureSampler::DeleteData()
{
	app->GetRenderer().UnLoadTextureSampler(this);
}

void Resources::TextureSampler::Write(Core::Serialization::Serializer& sr)
{
	sr.Write(static_cast<u8>(ObjectType::TextureSamplerType));
	IResource::Write(sr);
	sr.Write(static_cast<u8>((u8)linear | ((u8)wrap << 1)));
}

void Resources::TextureSampler::Load(Core::Serialization::Deserializer& dr)
{
	IResource::Load(dr);
	u8 mask = 0;
	dr.Read(mask);
	linear = mask & 0x1;
	wrap = mask & 0x2;
	app->GetRenderer().LoadTextureSampler(this, linear, wrap);
	isLoaded = true;
}

void Resources::TextureSampler::WindowCreateResource(bool& open)
{
	if (!prevbool && open)
		newTextureS = app->GetResources().CreateResource<TextureSampler>("newTextureSampler");
	prevbool = open;
	if (open)
	{
		Wrappers::Interfacing* gui = &app->GetInterfacing();
		gui->OpenPopup("Create Material");
		if (gui->BeginPopupModal("Create Material"))
		{
			static std::string tempName;
			gui->InputText("name", tempName);

			gui->CheckBox("wrap", newTextureS->wrap);
			gui->CheckBox("linear", newTextureS->linear);

			if (gui->Button("create"))
			{
				newTextureS->path = tempName;
				if (newTextureS->path.size() > 0)
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
				if (!newTextureS->isLoaded)
					app->GetResources().GetTextureSamplerList().pop_back();
				gui->CloseCurrentPopup();
				open = false;
			}
			gui->EndPopup();
		}
	}
}

TextureSampler* TextureSampler::GetDefaultSampler()
{
	return defaultSampler;
}

void TextureSampler::SetDefaultSampler()
{
	defaultSampler = this;
}
