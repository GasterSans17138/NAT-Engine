#pragma once

#include <string>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include "Maths/Maths.hpp"
#include <bitset>

namespace Wrappers
{
	enum InputEnableOptions
	{
		Keyboard = 0,
		Typing,
		MouseButton,
		MouseScroll,
		FileDrop,
		Size
	};

	enum PopupParam : u32
	{
		BUTTON_OK = 0x0,
		BUTTON_OK_CANCEL = 0x1,
		BUTTON_ABORT_RETRY_IGNORE = 0x2,
		BUTTON_YES_NO_CANCEL = 0x3,
		BUTTON_YES_NO = 0x4,
		BUTTON_RETRY_CANCEL = 0x5,
		BUTTON_CANCEL_RETRY_CONTINUE = 0x6,
		BUTTON_HELP = 0x4000,
		ICON_WARNING = 0x30,
		ICON_INFO = 0x40,
		ICON_QUESTION = 0x20,
		ICON_STOP = 0x10,
	};

	struct CallbackValues
	{
		std::bitset<InputEnableOptions::Size> options;
		std::bitset<2 * (GLFW_KEY_LAST + 1)> keyStates;
		std::bitset<2 * GLFW_MOUSE_BUTTON_LAST> mouseStates;
		std::vector<u16> changedKeys;
		std::vector<u16> changedButtons;
		std::vector<std::string> filePaths;
		std::u32string typedText;
		s32 lastKey = -1;
		s32 lastButton = -1;
		f32 mouseDelta = 0;
	};

	class NAT_API WindowManager 
	{
	public:
		WindowManager() = default;

		~WindowManager();

		bool CreateWindow(const std::string& name, Maths::IVec2 defaultResolution);
		void* GetWindowPointer();
		GLFWwindow* GetGLFWindow();
		void* GetWindowPtr();
		Maths::IVec2 GetWindowSize() const;
		Maths::IVec2 GetWindowPos();
		void Update();
		bool ShouldClose();
		void SetShouldClose(bool shouldClose);
		bool CreateSurface(void* instance, void* surface);
		void SetupCallbacks();
		void SetFullScreen(bool fullscreen);
		void SetTargetFrameRate(f32 value);

		std::vector<f32> GetAllScreenMaxFrameRates();

		bool IsKeyDown(u16 key);
		bool IsKeyPressed(u16 key);
		s16 GetLastKey();
		void ClearLastKey();
		const std::u32string& GetTypedText();
		void ClearTypedText();
		bool IsMouseButtonDown(u16 button);
		bool IsMouseButtonPressed(u16 button);
		s16 GetLastMouseButton();
		void ClearLastMouseButton();
		f32 GetMouseScrollDelta();
		const std::vector<std::string>& GetDroppedFiles();
		void ClearDroppedFiles();
		const char* GetKeyName(s16 key);
		void SetCursorCaptured(bool captured);
		std::string GetClipboardString();
		void SetClipboardString(const std::string& input);
		
		void SetKeyboardEnable(bool enable);
		void SetTypingEnable(bool enable);
		void SetMouseButtonEnable(bool enable);
		void SetMouseScrollEnable(bool enable);
		void SetFileDropEnable(bool enable);

		bool IsVisible()const;

		bool IsCursorCaptured() const { return mouseCaptured; };
		Maths::IVec2 GetCursorPos() const { return cursorPos; }
		Maths::Vec2 GetCursorDelta() const { return cursorDelta; }
		bool IsFullScreen() const { return isFullScreen; }
		f64 GetWindowTime() const { return windowTime; }
		f32 GetDeltaTime() const { return deltaTime; }
		static void ExecuteCommand(const std::string& command, const std::string& args = "");
		static s32 OpenPopup(const std::string& title, const std::string& message, PopupParam params);
		static s32 OpenPopup(const std::string& title, const std::string& message, u32 params);
		static void ErrorSound();
	private:
		GLFWwindow* window = nullptr;
		CallbackValues cbv = {};
	protected:
		f64 windowTime = 0;
		f32 deltaTime = 0;
		f32 targetDT = (1/60.0f);
		f32 cumulativeDelta = 0.0f;
		bool mouseCaptured = false;
		bool isFullScreen = false;
		Maths::IVec2 cursorPos;
		Maths::Vec2 cursorDelta;
		Maths::IVec2 savedSize;
		Maths::IVec2 savedPos;
	};

}