#pragma once

#include <functional>

#if Z_WINDOWS
#include "../../../headers/MSWin.h"
#elif Z_ANDROID
#include <android/input.h>
#endif

namespace zzz::engine
{
	class IWindow;

#pragma region Type Aliases
#if Z_WINDOWS
	struct WinMsg
	{
		HWND	hWnd;
		UINT	uMsg;
		WPARAM	wParam;
		LPARAM	lParam;
	};
	using NativeMsg = WinMsg;
#elif Z_ANDROID
	struct AndroidMsg
	{
		void* motionEvent;
		void* keyEvent;
	};
	using NativeMsg = AndroidMsg;
#elif Z_LINUX
	struct LinuxMsg
	{
		void* msg;
	};
	using NativeMsg = LinuxMsg;
#elif Z_MACOS
	struct MacOSMsg
	{
		void* msg;
	};
	using NativeMsg = MacOSMsg;
#elif Z_IOS
	struct iOSMsg
	{
		void* msg;
	};
	using NativeMsg = iOSMsg;
#else
#error ">>>>> Unsupported platform"
#endif
#pragma endregion

	class InputBase
	{
	public:
		std::function<void(int)> OnKeyDown;
		std::function<void(int)> OnKeyUp;
		std::function<void(int, int)> OnMouseMove;

	protected:
		InputBase() = default;
		~InputBase() = default;
	};
}
