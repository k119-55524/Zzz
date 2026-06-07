#pragma once

#include "../core/config/ConfigManager.h"
#include "../platforms/platforms/IPlatform.h"
#include "../platforms/native_view/window/WinLinux.h"
#include "../platforms/native_view/window/WinMSWindows.h"
#include "../platforms/native_view/window/WinAndroid.h"

namespace zzz::engine
{
#if defined(Z_WINDOWS)
	using Window = WinMSWindows;
#elif defined(Z_LINUX)
	using Window = WinLinux;
#elif defined(Z_ANDROID)
	using Window = WinAndroid;
#else
#error >>>>> Unsupported platform. No window implementation available.
#endif

	class PlatformFactory final
	{
	public:
		std::shared_ptr<IWindow> CreateAppWin(const std::shared_ptr<IPlatform> platform);
	};
}