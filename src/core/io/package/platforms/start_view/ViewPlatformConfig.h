#pragma once

#if Z_EDITOR || Z_WINDOWS
#include "ViewDataMSWin.h"
#elif Z_LINUX
#include "ViewDataLinux.h"
#elif Z_ANDROID
#include "ViewDataAndroid.h"
#elif Z_MACOS
#include "ViewDataMacOS.h"
#elif Z_IOS
#include "ViewDataiOS.h"
#else
#error >>>>> ViewPlatformConfig header inclusion: Unsupported platform.
#endif

namespace zzz::core
{
#if Z_EDITOR || Z_WINDOWS
	using ViewPlatformData = ViewDataMSWin;
#elif Z_LINUX
	using ViewPlatformData = ViewDataLinux;
#elif Z_ANDROID
	using ViewPlatformData = ViewDataAndroid;
#elif Z_MACOS
	using ViewPlatformData = ViewDataMacOS;
#elif Z_IOS
	using ViewPlatformData = ViewDataiOS;
#else
#error >>>>> ViewPlatformData alias: Unsupported platform.
#endif
}
