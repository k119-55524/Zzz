#pragma once

#include <string>
#include <expected>

#if defined(Z_WINDOWS)
#include "../../../headers/MSWin.h"
#elif defined(Z_ANDROID)
#include <android/input.h>
#endif

namespace zzz::engine
{
	class IWindow;

#pragma region Type Aliases
#if defined(Z_WINDOWS)
	struct WinMsg
	{
		UINT   uMsg;
		WPARAM wParam;
		LPARAM lParam;
	};
	using NativeMsg = WinMsg;
#elif defined(Z_ANDROID)
	struct AndroidMsg
	{
		void* motionEvent;
		void* keyEvent;
	};
	using NativeMsg = AndroidMsg;
#elif defined(Z_LINUX)
	struct LinuxMsg
	{
		void* msg;
	};
	using NativeMsg = LinuxMsg;
#elif defined(Z_MACOS)
	struct MacOSMsg
	{
		void* msg;
	};
	using NativeMsg = MacOSMsg;
#elif defined(Z_IOS)
	struct iOSMsg
	{
		void* msg;
	};
	using NativeMsg = iOSMsg;
#else
#error ProcessMessage
#endif
#pragma endregion

	}
