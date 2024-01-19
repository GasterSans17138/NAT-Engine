#include "Resources/Sound.hpp"

#include "Resources/ResourceManager.hpp"
#include "Resources/SoundData.hpp"
#include "Core/App.hpp"

using namespace Resources;

const char* const attenuationTypes[] = 
{
	"None",
	"Inverse",
	"Linear",
	"Exponential"
};

Sound::Sound()
{
	canBeEdited = true;
}

void Sound::DeleteData()
{
	app->GetSoundLoader().UnLoadSound(this);
	app->GetResources().Release(data);
}

bool Sound::ShowEditWindow()
{
	auto& gui = app->GetInterfacing();
	bool open = IResource::BeginEditWindow(gui);
	auto& engine = app->GetSoundEngine();
	if (gui.SoundDataListCombo(&data))
	{
		auto& loader = app->GetSoundLoader();
		loader.UnLoadSound(this);
		if (data) loader.LoadSound(this);
	}
	if (!engine.IsPlaying(this))
	{
		if (gui.Button("Play sound"))
		{
			engine.StartSound(this);
			engine.ResetSound(this);
		}
	}
	else
	{
		if (gui.Button("Stop sound")) engine.StopSound(this);
	}
	bool looped = engine.IsLooping(this);
	if (gui.CheckBox("Looping", &looped))
	{
		engine.SetLooping(this, looped);
		isSaved = false;
	}
	looped = engine.IsSpatializationEnabled(this);
	if (gui.CheckBox("Use Spatialization", &looped))
	{
		engine.SetSpatializationEnabled(this, looped);
		isSaved = false;
	}
	f32 number = engine.GetVolumeLinear(this);
	if (gui.SliderFloat("Volume", &number, 0.0f, 2.0f, false))
	{
		engine.SetVolumeLinear(this, number);
		isSaved = false;
	}
	number = engine.GetPan(this);
	if (gui.SliderFloat("Pan", &number, -1.0f, 1.0f, false))
	{
		engine.SetPan(this, number);
		isSaved = false;
	}
	number = engine.GetPitch(this);
	if (gui.SliderFloat("Pitch", &number, 0.001f, 10.0f, false))
	{
		engine.SetPitch(this, number);
		isSaved = false;
	}
	number = engine.GetDopplerFactor(this);
	if (gui.SliderFloat("Doppler factor", &number, 0.0f, 10.0f, false))
	{
		engine.SetDopplerFactor(this, number);
		isSaved = false;
	}
	number = engine.GetRollOff(this);
	if (gui.SliderFloat("Roll-off", &number, 0.0f, 10.0f, false))
	{
		engine.SetRollOff(this, number);
		isSaved = false;
	}
	s32 type = static_cast<s32>(engine.GetAttenuationType(this));
	if (gui.Combo("Attenuation type", &type, attenuationTypes, 4))
	{
		engine.SetAttenuationType(this, static_cast<Wrappers::Sound::AttenuationType>(type));
		isSaved = false;
	}
	Maths::Vec2 bounds = engine.GetDistances(this);
	if (gui.DragFloat2("Attenuation bounds", &bounds.x, 0.1f, 0.0f, INFINITY))
	{
		engine.SetDistances(this, bounds);
		isSaved = false;
	}
	bounds = engine.GetGains(this);
	if (gui.DragFloat2("Gain Values", &bounds.x, 0.1f, 0.0f, INFINITY))
	{
		engine.SetGains(this, bounds);
		isSaved = false;
	}
	return IResource::EndEditWindow(gui) || !open;
}

void Sound::Write(Core::Serialization::Serializer& sr)
{
	sr.Write(static_cast<u8>(ObjectType::SoundType));
	IResource::Write(sr);
	sr.Write(data ? data->hash : (u64)0);
	if (data)
	{
		auto& engine = app->GetSoundEngine();
		u8 mask = static_cast<u8>(engine.IsLooping(this)) | static_cast<u8>(engine.IsSpatializationEnabled(this) << 1);
		sr.Write(mask);
		sr.Write(engine.GetVolumeLinear(this));
		sr.Write(engine.GetPan(this));
		sr.Write(engine.GetPitch(this));
		sr.Write(engine.GetDopplerFactor(this));
		sr.Write(engine.GetRollOff(this));
		sr.Write(static_cast<u8>(engine.GetAttenuationType(this)));
		sr.Write(engine.GetDistances(this));
		sr.Write(engine.GetGains(this));
	}
}

void Sound::Load(Core::Serialization::Deserializer& dr)
{
	IResource::Load(dr);
	u64 hash = 0;
	dr.Read(hash);
	if (hash) data = app->GetResources().Get<SoundData>(hash);
	app->GetSoundLoader().LoadSound(this);
	if (data)
	{
		auto& engine = app->GetSoundEngine();
		u8 mask = 0;
		dr.Read(mask);
		engine.SetLooping(this, mask & 0x1);
		engine.SetSpatializationEnabled(this, mask & 0x2);
		f32 number = 0.0f;
		dr.Read(number);
		engine.SetVolumeLinear(this, number);
		number = 0.0f;
		dr.Read(number);
		engine.SetPan(this, number);
		number = 0.0f;
		dr.Read(number);
		engine.SetPitch(this, number);
		number = 0.0f;
		dr.Read(number);
		engine.SetDopplerFactor(this, number);
		number = 0.0f;
		dr.Read(number);
		engine.SetRollOff(this, number);
		u8 att = 0;
		dr.Read(att);
		engine.SetAttenuationType(this, static_cast<Wrappers::Sound::AttenuationType>(att));
		Maths::Vec2 bounds;
		dr.Read(bounds);
		engine.SetDistances(this, bounds);
		bounds = Maths::Vec2();
		dr.Read(bounds);
		engine.SetGains(this, bounds);
	}
}

void Sound::WindowCreateResource(bool& open)
{
	if (!prevbool && open)
		newSound = app->GetResources().CreateResource<Sound>("newSound");
	prevbool = open;
	if (open)
	{
		Wrappers::Interfacing* gui = &app->GetInterfacing();
		gui->OpenPopup("Create Sound");
		if (gui->BeginPopupModal("Create Sound"))
		{
			static std::string tempName;
			gui->InputText("name", tempName);

			auto& engine = app->GetSoundEngine();
			if (gui->SoundDataListCombo(&newSound->data))
			{
				auto& loader = app->GetSoundLoader();
				loader.UnLoadSound(this);
				if (newSound->data)
				{
					newSound->isLoaded = loader.LoadSound(newSound);
				}
			}
			if (newSound->data)
			{
				if (!engine.IsPlaying(newSound))
				{
					if (gui->Button("Play sound")) engine.StartSound(newSound);
				}
				else
				{
					if (gui->Button("Stop sound")) engine.StopSound(newSound);
				}
				bool looped = engine.IsLooping(newSound);
				if (gui->CheckBox("Looping", &looped))
				{
					engine.SetLooping(newSound, looped);
				}
				looped = engine.IsSpatializationEnabled(newSound);
				if (gui->CheckBox("Use Spatialization", &looped))
				{
					engine.SetSpatializationEnabled(newSound, looped);
				}
			}

			if (gui->Button("Create"))
			{
				newSound->path = tempName;
				if (newSound->path.size() > 0 && newSound->data)
				{
					newSound->isLoaded = true;
					gui->CloseCurrentPopup();
					open = false;
					tempName = "";
				}
				else
				{
					if (newSound->path.size() <= 0)
					{
						LOG(DEBUG_LEVEL::LDEFAULT, "Your resource need a name");
					}
					else
					{
						LOG(DEBUG_LEVEL::LDEFAULT, "Your resource need a sound data !");
					}
				}
			}


			if (gui->Button("Close"))
			{
				if (!newSound->isLoaded)
					app->GetResources().GetSoundList().pop_back();
				gui->CloseCurrentPopup();
				open = false;
			}
			gui->EndPopup();

		}

	}
}
