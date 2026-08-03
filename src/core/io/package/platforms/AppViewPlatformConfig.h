#pragma once

#if Z_EDITOR || Z_WINDOWS
#include "AppViewDataMSWin.h"
#elif Z_LINUX
#include "AppViewDataLinux.h"
#elif Z_ANDROID
#include "AppViewDataAndroid.h"
#elif Z_MACOS
#include "AppViewDataMacOS.h"
#elif Z_IOS
#include "AppViewDataiOS.h"
#else
#error >>>>> AppViewPlatformConfig header inclusion: Unsupported platform.
#endif

namespace zzz::core
{
#if Z_EDITOR || Z_WINDOWS
	using AppViewPlatformData = AppViewDataMSWin;
#elif Z_LINUX
	using AppViewPlatformData = AppViewDataLinux;
#elif Z_ANDROID
	using AppViewPlatformData = AppViewDataAndroid;
#elif Z_MACOS
	using AppViewPlatformData = AppViewDataMacOS;
#elif Z_IOS
	using AppViewPlatformData = AppViewDataiOS;
#else
#error >>>>> AppViewPlatformData alias: Unsupported platform.
#endif
}
