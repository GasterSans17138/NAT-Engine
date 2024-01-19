#include "LowRenderer/PostProcess/PostProcessManager.hpp"

#include "Core/App.hpp"
#include "LowRenderer/PostProcess/GammaCorrectionPostProcess.hpp"
#include "LowRenderer/PostProcess/BloomPostProcess.hpp"
#include "LowRenderer/PostProcess/BlurPostProcess.hpp"

using namespace LowRenderer::PostProcess;

PostProcessManager::~PostProcessManager()
{
	for (auto& pass : effects)
	{
		delete pass;
	}
	for (auto& pass : allEffects)
	{
		delete pass;
	}
}

void PostProcessManager::ApplyEffects(LowRenderer::FrameBuffer* fb)
{
	if (!lightShader) lightShader = Core::App::GetInstance()->GetResources().Get<Resources::ShaderProgram>(0x20);
	if (!renderer) renderer = &Core::App::GetInstance()->GetRenderer();
	if (fb) fb->ResetBuffer();
	else renderer->ResetMainFB();
	renderer->BeginPass(fb, LowRenderer::RenderPassType::LIGHT);
	renderer->ApplyLightPass(lightShader);
	renderer->EndPass();
	for (auto& pass : effects)
	{
		pass->ApplyPass(renderer, fb);
	}
}

void PostProcessManager::ShowGUI()
{
	if (guiInterface) guiInterface = &Core::App::GetInstance()->GetInterfacing();

	//Draw all active effects
	for (u64 i = 0; i < effects.size(); i++)
	{
		guiInterface->PushId((s32)i);
		bool visible = true;
		bool up = guiInterface->ArrowButton("Up", 2);
		guiInterface->SameLine();
		bool down = guiInterface->ArrowButton("Down", 3);
		guiInterface->SameLine();
		guiInterface->SetCursorX(60);
		bool open = guiInterface->CollapsingHeader(effects[i]->GetEffectName(), &visible);
		if (!visible)
		{
			delete effects[i];
			for (u64 j = i; j < effects.size() - 1; j++)
			{
				effects[j] = effects[j + 1];
			}
			effects.pop_back();
		}
		else
		{
			if (open) effects[i]->Configure();
			if (up && i)
			{
				std::swap(effects[i], effects[i - 1]);
			}
			else if (down && i < effects.size() - 1)
			{
				std::swap(effects[i], effects[i + 1]);
			}
		}
		guiInterface->PopId();
	}

	//Add Effect Dropdown
	if (!showEffectListDropdown && guiInterface->Button("Add Effect"))
	{
		showEffectListDropdown = true;
	}

	else if (showEffectListDropdown && guiInterface->BeginCombo("###PostProcessDropDown", "New effect", 0))
	{
		for (int i = 0; i < effectsNameList.size(); i++)
		{
			if(guiInterface->Selectable(effectsNameList[i]))
				effects.push_back(allEffects[i]->CreateCopy());
		}

		guiInterface->EndCombo();
	}
}

void PostProcessManager::Init()
{
	allEffects.push_back(new GammaCorrectionPostProcess());
	allEffects.push_back(new BloomPostProcess());
	allEffects.push_back(new BlurPostProcess());

	effectsNameList.reserve(allEffects.size());

	for (auto& effect : allEffects)
		effectsNameList.push_back(effect->GetEffectName());
}

void PostProcessManager::Serialize(Core::Serialization::Serializer& sr)
{
	sr.Write(effects.size());
	for (u64 i = 0; i < effects.size(); ++i)
	{
		sr.Write(static_cast<u8>(effects[i]->GetType()));
		effects[i]->Serialize(sr);
	}
}

void PostProcessManager::Deserialize(Core::Serialization::Deserializer& dr)
{
	u64 size = 0;
	if (!dr.Read(size)) return;
	effects.reserve(size);
	EffectType type;
	for (u64 i = 0; i < size; ++i)
	{
		if (!dr.Read(reinterpret_cast<u8&>(type))) return;
		switch (type)
		{
		case LowRenderer::PostProcess::EffectType::BLOOM:
			effects.push_back(new BloomPostProcess());
			break;
		case LowRenderer::PostProcess::EffectType::BLUR:
			effects.push_back(new BlurPostProcess());
			break;
		case LowRenderer::PostProcess::EffectType::GAMMA_CORRECTION:
			effects.push_back(new GammaCorrectionPostProcess());
			break;
		default:
			LOG(DEBUG_LEVEL::LERROR, "Unknown post process effect type %d !", type);
			return;
		}
		effects.back()->Deserialize(dr);
	}
}
