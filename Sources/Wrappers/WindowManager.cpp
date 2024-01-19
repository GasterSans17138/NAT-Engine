#include "Wrappers/WindowManager.hpp"

#include "Renderer/RendererTexture.hpp"
#include "Core/Debugging/Log.hpp"
#include "Wrappers/ImageLoader.hpp"

#pragma warning(disable : 26812)

using namespace Wrappers;

#define ICON_PATH "Default_Resources/Icon/Icon_64.png"

WindowManager::~WindowManager()
{
	glfwDestroyWindow(window);
	glfwTerminate();
}

bool WindowManager::CreateWindow(const std::string& name, Maths::IVec2 defaultResolution)
{
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
	window = glfwCreateWindow(defaultResolution.x, defaultResolution.y, name.c_str(), nullptr, nullptr);
	if (!window)
	{
		return false;
	}
	GLFWimage icon = {};
	s32 texChannels;
	icon.pixels = ImageLoader::LoadStbi(ICON_PATH, &icon.width, &icon.height, &texChannels);
	if (icon.pixels)
	{
		glfwSetWindowIcon(window, 1, &icon);
		ImageLoader::FreeStbi(icon.pixels);
	}
	else
	{
		LOG(DEBUG_LEVEL::LWARNING, "Could not load icon " ICON_PATH);
	}
	if (glfwRawMouseMotionSupported())
	{
		glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
		LOG_DEFAULT("Enabling raw cursor input mode");
	}
	return true;
}

void* WindowManager::GetWindowPointer()
{
	return window;
}

GLFWwindow* Wrappers::WindowManager::GetGLFWindow()
{
	return window;
}


Maths::IVec2 Wrappers::WindowManager::GetWindowSize() const
{
	Maths::IVec2 result;
	glfwGetWindowSize(window, &result.x, &result.y);
	return result;
}

Maths::IVec2 Wrappers::WindowManager::GetWindowPos()
{
	Maths::IVec2 result;
	glfwGetWindowPos(window, &result.x, &result.y);
	return result;
}

void WindowManager::Update()
{
	for (auto& key : cbv.changedKeys)
	{
		cbv.keyStates.reset(2llu * key);
	}
	cbv.changedKeys.clear();
	for (auto& button : cbv.changedButtons)
	{
		cbv.mouseStates.reset(2llu * button);
	}
	cbv.changedButtons.clear();
	cbv.mouseDelta = 0.0f;
	glfwPollEvents();
	f64 newTime = glfwGetTime();
	deltaTime = (f32)(newTime - windowTime);
	windowTime = newTime;

	if (!isFullScreen && Maths::Util::IsNear(deltaTime, targetDT, 0.05f)) // Sussy deltatime correction is only useful when in windowed
	{
		cumulativeDelta += deltaTime - targetDT;
		deltaTime = targetDT;
		if (std::abs(cumulativeDelta) > 0.1f)
		{
			deltaTime += cumulativeDelta;
			if (deltaTime < 0.001f) deltaTime = 0.01666f; // HOW THE F DOS THAT DELTATIME GOES NEGATIVE FROM ONE BUILD TO ANOTHER I SWEAR IM GONNA <ysdfpgihsjàadpcjiadfbuivk
			cumulativeDelta = 0.0f;
		}
	}

	f64 x, y;
	glfwGetCursorPos(window, &x, &y);
	cursorDelta.x = static_cast<f32>(x - cursorPos.x);
	cursorDelta.y = static_cast<f32>(y - cursorPos.y);
	cursorPos.x = static_cast<s32>(floor(x));
	cursorPos.y = static_cast<s32>(floor(y));
}

bool WindowManager::ShouldClose()
{
	return glfwWindowShouldClose(window);
}

void Wrappers::WindowManager::SetShouldClose(bool shouldClose)
{
	glfwSetWindowShouldClose(window, shouldClose);
}

bool WindowManager::CreateSurface(void* instance, void* surface)
{
	return glfwCreateWindowSurface(*static_cast<VkInstance*>(instance), window, nullptr, static_cast<VkSurfaceKHR*>(surface)) == VK_SUCCESS;
}

void CharCallback(GLFWwindow* window, unsigned int codepoint)
{
	auto values = reinterpret_cast<CallbackValues*>(glfwGetWindowUserPointer(window));
	if (!values->options.test(InputEnableOptions::Typing)) return;
	values->typedText.push_back(codepoint);
}

void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key < 0) return;
	auto values = reinterpret_cast<CallbackValues*>(glfwGetWindowUserPointer(window));
	if (!values->options.test(InputEnableOptions::Keyboard)) return;
	if (action == GLFW_PRESS)
	{
		values->keyStates.set(2llu * key);
		values->keyStates.set(2llu * key + 1);
		values->changedKeys.push_back(key);
		values->lastKey = key;
	}
	else if (action == GLFW_RELEASE)
	{
		values->keyStates.reset(2llu * key);
		values->keyStates.reset(2llu * key + 1);
	}
}

void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
	if (button < 0) return;
	auto values = reinterpret_cast<CallbackValues*>(glfwGetWindowUserPointer(window));
	if (!values->options.test(InputEnableOptions::MouseButton)) return;
	if (action == GLFW_PRESS)
	{
		values->mouseStates.set(2llu * button);
		values->mouseStates.set(2llu * button + 1);
		values->changedButtons.push_back(button);
		values->lastButton = button;
	}
	else if (action == GLFW_RELEASE)
	{
		values->mouseStates.reset(2llu * button);
		values->mouseStates.reset(2llu * button + 1);
	}
}

void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
	(void)xoffset;
	auto values = reinterpret_cast<CallbackValues*>(glfwGetWindowUserPointer(window));
	if (!values->options.test(InputEnableOptions::MouseScroll)) return;
	values->mouseDelta += static_cast<f32>(yoffset);
}

void DropCallback(GLFWwindow* window, int count, const char** paths)
{
	auto values = reinterpret_cast<CallbackValues*>(glfwGetWindowUserPointer(window));
	if (!values->options.test(InputEnableOptions::FileDrop)) return;
	values->filePaths.clear();
	values->filePaths.resize(count);
	for (s32 i = 0; i < count; i++)
	{
		values->filePaths[i] = paths[i];
	}
}


void WindowManager::SetupCallbacks()
{
	glfwSetWindowUserPointer(window, &cbv);
	glfwSetCharCallback(window, &CharCallback);
	glfwSetKeyCallback(window, &KeyCallback);
	glfwSetMouseButtonCallback(window, &MouseButtonCallback);
	glfwSetScrollCallback(window, &ScrollCallback);
	glfwSetDropCallback(window, &DropCallback);
}

void Wrappers::WindowManager::SetFullScreen(bool fullscreen)
{
	if (fullscreen == isFullScreen) return;
	if (fullscreen)
	{
		glfwGetWindowPos(window, &savedPos.x, &savedPos.y);
		glfwGetWindowSize(window, &savedSize.x, &savedSize.y);
		GLFWmonitor* monitor = glfwGetPrimaryMonitor();
		const GLFWvidmode* mode = glfwGetVideoMode(monitor);
		glfwSetWindowMonitor(window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
	}
	else
	{
		glfwSetWindowMonitor(window, NULL, savedPos.x, savedPos.y, savedSize.x, savedSize.y, GLFW_DONT_CARE);
	}
	isFullScreen = fullscreen;
}

std::vector<f32> WindowManager::GetAllScreenMaxFrameRates()
{
	std::vector<f32> result;
	s32 count = 0;
	auto monitors = glfwGetMonitors(&count);
	if (!monitors || count <= 0) return result;
	result.resize(count);
	for (s32 i = 0; i < count; i++)
	{
		auto stuff = glfwGetVideoMode(monitors[i]);
		result[i] = std::roundf(stuff->refreshRate / 5.0f) * 5.0f;
	}
	return result;
}

void WindowManager::SetTargetFrameRate(f32 value)
{
	targetDT = 1/value;
}

bool WindowManager::IsKeyDown(u16 key)
{
	return cbv.keyStates.test(2llu * key + 1);
}

bool WindowManager::IsKeyPressed(u16 key)
{
	return cbv.keyStates.test(2llu * key);
}

s16 WindowManager::GetLastKey()
{
	return cbv.lastKey;
}

void WindowManager::ClearLastKey()
{
	cbv.lastKey = -1;
}

const std::u32string& WindowManager::GetTypedText()
{
	return cbv.typedText;
}

void WindowManager::ClearTypedText()
{
	cbv.typedText.clear();
}

bool WindowManager::IsMouseButtonDown(u16 button)
{
	return cbv.mouseStates.test(2llu * button + 1);
}

bool WindowManager::IsMouseButtonPressed(u16 button)
{
	return cbv.mouseStates.test(2llu * button);
}

s16 WindowManager::GetLastMouseButton()
{
	return cbv.lastButton;
}

void WindowManager::ClearLastMouseButton()
{
	cbv.lastButton = -1;
}

f32 WindowManager::GetMouseScrollDelta()
{
	return cbv.mouseDelta;
}

const std::vector<std::string>& WindowManager::GetDroppedFiles()
{
	return cbv.filePaths;
}

void WindowManager::ClearDroppedFiles()
{
	cbv.filePaths.clear();
}

void WindowManager::SetKeyboardEnable(bool enable)
{
	cbv.options.set(InputEnableOptions::Keyboard, enable);
}

void WindowManager::SetTypingEnable(bool enable)
{
	cbv.options.set(InputEnableOptions::Typing, enable);
}

void WindowManager::SetMouseButtonEnable(bool enable)
{
	cbv.options.set(InputEnableOptions::MouseButton, enable);
}

void WindowManager::SetMouseScrollEnable(bool enable)
{
	cbv.options.set(InputEnableOptions::MouseScroll, enable);
}

void WindowManager::SetFileDropEnable(bool enable)
{
	cbv.options.set(InputEnableOptions::FileDrop, enable);
}

bool Wrappers::WindowManager::IsVisible() const
{
	Maths::IVec2 size = GetWindowSize();
	return !(size.x == 0 && size.y == 0);
}

const char* WindowManager::GetKeyName(s16 key)
{
	return glfwGetKeyName(key, 0);
}

void WindowManager::SetCursorCaptured(bool captured)
{
	if (captured == mouseCaptured) return;
	if (captured && !mouseCaptured)
	{
		mouseCaptured = true;
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	}
	else if (!captured && mouseCaptured)
	{
		mouseCaptured = false;
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	}
}

std::string WindowManager::GetClipboardString()
{
	std::string result = glfwGetClipboardString(window);
	return result;
}

void WindowManager::SetClipboardString(const std::string& input)
{
	glfwSetClipboardString(window, input.c_str());
}

#ifdef _WIN32
#include <Windows.h>
#define GLFW_EXPOSE_NATIVE_WIN32 1
#include <GLFW/glfw3native.h>
#endif

void WindowManager::ExecuteCommand(const std::string& command, const std::string& args)
{
#ifdef _WIN32
	std::wstring prog(command.begin(), command.end());
	std::wstring param(args.begin(), args.end());
	SHELLEXECUTEINFO ShExecInfo = { 0 };
	ShExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
	ShExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
	ShExecInfo.hwnd = NULL;
	ShExecInfo.lpVerb = NULL;
	ShExecInfo.lpFile = prog.c_str();
	ShExecInfo.lpParameters = param.c_str();
	ShExecInfo.lpDirectory = NULL;
	ShExecInfo.nShow = SW_HIDE;
	ShExecInfo.hInstApp = NULL;
	ShellExecuteEx(&ShExecInfo);
	if (!ShExecInfo.hProcess)
	{
		LOG(DEBUG_LEVEL::LERROR, "Something went wrong, could not execute command !");
		return;
	}
	WaitForSingleObject(ShExecInfo.hProcess, INFINITE);
	CloseHandle(ShExecInfo.hProcess);
#else
	system(command.c_str()); // TODO better
#endif

}

s32 WindowManager::OpenPopup(const std::string& title, const std::string& message, PopupParam params)
{
	std::wstring t(title.begin(), title.end());
	std::wstring m(message.begin(), message.end());
	return MessageBox(nullptr, m.c_str(), t.c_str(), params);
}
const u32 G2 = 98;
const u32 C3 = 131;
const u32 BF4 = 466;
const u32 C5 = 523;
const u32 D5 = 587;
const u32 EF5 = 622;
const u32 F5 = 698;
const u32 GF5 = 740;

s32 WindowManager::OpenPopup(const std::string& title, const std::string& message, u32 params)
{
	return OpenPopup(title, message, static_cast<PopupParam>(params));
}

void WindowManager::ErrorSound()
{
	Beep(C5, 400);
	Beep(EF5, 400);
	Beep(F5, 400);
	Beep(GF5, 400);
	Beep(F5, 400);
	Beep(EF5, 400);
	Beep(C5, 400);
	Beep(0, 600);
	Beep(BF4, 200);
	Beep(D5, 200);
	Beep(C5, 200);
}

void* WindowManager::GetWindowPtr()
{
	return glfwGetWin32Window(window);
}