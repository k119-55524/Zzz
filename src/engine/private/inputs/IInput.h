#pragma once

#include <string>
#include <expected>

#if defined(Z_WINDOWS)
#include "../../headers/MSWin.h"
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
		AInputEvent* event;
	};
	using NativeMsg = AndroidMsg;
#elif defined(Z_LINUX)
	struct LinuxMsg
	{
		void* event;
	};
	using NativeMsg = LinuxMsg;
#elif defined(Z_MACOS)
	struct MacOSMsg
	{
		void* event;
	};
	using NativeMsg = MacOSMsg;
#elif defined(Z_IOS)
	struct iOSMsg
	{
		void* event;
	};
	using NativeMsg = iOSMsg;
#else
#error ProcessMessage
#endif
#pragma endregion

	class IInput
	{
		Z_NO_COPY_MOVE(IInput);

	public:
		IInput() = default;
		virtual ~IInput() = default;

		[[nodiscard]] virtual std::expected<void, std::string> Initialize() = 0;
		virtual bool ProcessMessage(const NativeMsg& nativeMsg) = 0;
	};
}
