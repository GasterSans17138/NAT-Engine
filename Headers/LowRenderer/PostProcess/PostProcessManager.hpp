#pragma once

#include <vector>

#include "PostProcessEffect.hpp"

namespace Renderer
{
	class VulkanRenderer;
}

namespace Wrappers
{
	class Interfacing;
}

namespace LowRenderer
{
	class FrameBuffer;

	namespace PostProcess
	{
		class NAT_API PostProcessManager
		{
		public:
	  		 PostProcessManager() = default;
			~PostProcessManager();

			void ApplyEffects(LowRenderer::FrameBuffer* fb);

			void ShowGUI();

			void Init();

			void Serialize(Core::Serialization::Serializer& sr);
			void Deserialize(Core::Serialization::Deserializer& dr);

			std::vector<PostProcessEffect*> effects;
		private:
			bool showEffectListDropdown = false;
			std::vector<const char*> effectsNameList;
			std::vector<PostProcessEffect*> allEffects;
			Resources::ShaderProgram* lightShader = nullptr;
			Renderer::VulkanRenderer* renderer = nullptr;
			Wrappers::Interfacing* guiInterface = nullptr;
		};
	}
}