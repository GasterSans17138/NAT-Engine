#pragma once

#include "App.hpp"

namespace Core
{

#define APP_NAME "NAT_Engine : Game"

	class GameApp final : public App
	{
	public :
		GameApp() = default;
		~GameApp() = default;

		void Init() override;
		void Run() override;
		void Destroy() override;

	private:
		void DefaultSamplerLoaded() override {}; //Nothing
	};
}

