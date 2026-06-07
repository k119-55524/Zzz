#pragma once

#include "../core/config/ConfigManager.h"
#include "../platforms/platforms/IPlatform.h"
#include "../platforms/native_view/window/WinLinux.h"
#include "../platforms/native_view/window/WinMSWindows.h"
#include "../platforms/native_view/window/WinAndroid.h"
#include "../platforms/main_loop/IMainLoop.h"
#include "../platforms/main_loop/MainLoop_MSWin.h"
#include "../platforms/main_loop/MainLoop_Linux.h"
#include "../platforms/main_loop/MainLoop_Android.h"

namespace zzz::engine
{
#if defined(Z_WINDOWS)
	using Window   = WinMSWindows;
	using MainLoop = MainLoop_MSWin;
#elif defined(Z_LINUX)
	using Window   = WinLinux;
	using MainLoop = MainLoop_Linux;
#elif defined(Z_ANDROID)
	using Window   = WinAndroid;
	using MainLoop = MainLoop_Android;
#else
#error >>>>> Unsupported platform. No window implementation available.
#endif

	class EngineFactory final
	{
	public:
		std::shared_ptr<IWindow>   CreateAppWin(const std::shared_ptr<IPlatform> platform);
		std::shared_ptr<IMainLoop> CreateMainLoop(std::shared_ptr<IPlatform> platform);
	};
}
