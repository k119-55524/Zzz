#pragma once

#if Z_WINDOWS
#include "ConfigMSWin.h"
#elif Z_LINUX
#include "ConfigLinux.h"
#elif Z_ANDROID
#include "ConfigAndroid.h"
#elif Z_MACOS
#include "ConfigMacOS.h"
#elif Z_IOS
#include "ConfigiOS.h"
#else
#error >>>>> Unsupported platform
#endif

namespace zzz::engine
{
#if Z_WINDOWS
	using PlatformConfig = ConfigMSWin;
#elif Z_LINUX
	using PlatformConfig = ConfigLinux;
#elif Z_ANDROID
	using PlatformConfig = ConfigAndroid;
#elif Z_MACOS
	using PlatformConfig = ConfigMacOS;
#elif Z_IOS
	using PlatformConfig = ConfigiOS;
#else
#error >>>>> Unsupported platform
#endif
}
