#pragma once

#include "App.hpp"

namespace Core
{

#define APP_NAME "NAT_Engine : Editor"

	class EditorApp final : public App
	{
	public:
		 EditorApp() = default;
		~EditorApp() = default;

		void Init()		override;
		void Run()		override;
		void Destroy()	override;

	private:
		void DefaultSamplerLoaded() override;
	};
};