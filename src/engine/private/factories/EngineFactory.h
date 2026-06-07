#pragma once

#include "../platforms/platforms/IPlatform.h"
#include "../core/config/platforms/IConfig.h"
#include "../platforms/native_view/window/WinLinux.h"
#include "../platforms/native_view/window/WinMSWindows.h"
#include "../platforms/native_view/window/WinAndroid.h"
#include "../platforms/main_loop/IMainLoop.h"
#include "../platforms/main_loop/MainLoop_MSWin.h"
#include "../platforms/main_loop/MainLoop_Linux.h"
#include "../platforms/main_loop/MainLoop_Android.h"
#include "../core/config/platforms/ConfigMSWin.h"
#include "../core/config/platforms/ConfigLinux.h"
#include "../core/config/platforms/ConfigAndroid.h"

namespace zzz::engine
{
#if defined(Z_WINDOWS)
	using Window          = WinMSWindows;
	using MainLoop        = MainLoop_MSWin;
	using PlatformConfig  = ConfigMSWin;
#elif defined(Z_LINUX)
	using Window          = WinLinux;
	using MainLoop        = MainLoop_Linux;
	using PlatformConfig  = ConfigLinux;
#elif defined(Z_ANDROID)
	using Window          = WinAndroid;
	using MainLoop        = MainLoop_Android;
	using PlatformConfig  = ConfigAndroid;
#else
#error >>>>> Unsupported platform. No window implementation available.
#endif

	class EngineFactory final
	{
	public:
		std::shared_ptr<IConfig>   CreatePlatformConfig();
		std::shared_ptr<IWindow>   CreateAppWin(const std::shared_ptr<IPlatform> platform);
		std::shared_ptr<IMainLoop> CreateMainLoop(std::shared_ptr<IPlatform> platform);
	};
}
