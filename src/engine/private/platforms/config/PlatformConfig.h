#pragma once

#if defined(Z_WINDOWS)
#include "ConfigMSWin.h"
#elif defined(Z_LINUX)
#include "ConfigLinux.h"
#elif defined(Z_ANDROID)
#include "ConfigAndroid.h"
#elif defined(Z_MACOS)
#include "ConfigMacOS.h"
#elif defined(Z_IOS)
#include "ConfigiOS.h"
#else
#error >>>>> Unsupported platform
#endif

namespace zzz::engine
{
#if defined(Z_WINDOWS)
	using PlatformConfig = ConfigMSWin;
#elif defined(Z_LINUX)
	using PlatformConfig = ConfigLinux;
#elif defined(Z_ANDROID)
	using PlatformConfig = ConfigAndroid;
#elif defined(Z_MACOS)
	using PlatformConfig = ConfigMacOS;
#elif defined(Z_IOS)
	using PlatformConfig = ConfigiOS;
#else
#error >>>>> Unsupported platform
#endif
}
