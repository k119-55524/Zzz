#pragma once

#pragma region Include
#include "../platforms/Platform.h"
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
#include "../core/config/platforms/ConfigMacOS.h"
#include "../core/config/platforms/ConfigiOS.h"
#include "../platforms/native_view/window/WinMacOS.h"
#include "../platforms/native_view/window/WiniOS.h"
#include "../platforms/main_loop/MainLoop_MacOS.h"
#include "../platforms/main_loop/MainLoop_iOS.h"
#include "../inputs/platforms/InputMSWindows.h"
#include "../inputs/platforms/InputAndroid.h"
#include "../inputs/platforms/InputLinux.h"
#include "../inputs/platforms/InputMacOS.h"
#include "../inputs/platforms/InputiOS.h"
#pragma endregion

#pragma region Type Aliases
namespace zzz::engine
{
#if defined(Z_WINDOWS)
	using Window          = WinMSWindows;
	using MainLoop        = MainLoop_MSWin;
	using PlatformConfig  = ConfigMSWin;
	using Input           = InputMSWindows;
#elif defined(Z_LINUX)
	using Window          = WinLinux;
	using MainLoop        = MainLoop_Linux;
	using PlatformConfig  = ConfigLinux;
	using Input           = InputLinux;
#elif defined(Z_ANDROID)
	using Window          = WinAndroid;
	using MainLoop        = MainLoop_Android;
	using PlatformConfig  = ConfigAndroid;
	using Input           = InputAndroid;
#elif defined(Z_MACOS)
	using Window          = WinMacOS;
	using MainLoop        = MainLoop_MacOS;
	using PlatformConfig  = ConfigMacOS;
	using Input           = InputMacOS;
#elif defined(Z_IOS)
	using Window          = WiniOS;
	using MainLoop        = MainLoop_iOS;
	using PlatformConfig  = ConfigiOS;
	using Input           = InputiOS;
#else
#error >>>>> Unsupported platform.
#endif
#pragma endregion

	class EngineFactory final
	{
	public:
		std::shared_ptr<IConfig>   CreatePlatformConfig();
		std::shared_ptr<IWindow>   CreateAppWin(const std::shared_ptr<Platform> platform, const std::shared_ptr<IInput> input);
		std::shared_ptr<IMainLoop> CreateMainLoop(std::shared_ptr<Platform> platform);
		std::shared_ptr<IInput>    CreateInput();
	};
}
