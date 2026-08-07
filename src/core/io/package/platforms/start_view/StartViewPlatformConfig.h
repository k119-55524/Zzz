#pragma once

#if Z_EDITOR || Z_WINDOWS
#include "StartViewDataMSWin.h"
#elif Z_LINUX
#include "StartViewDataLinux.h"
#elif Z_ANDROID
#include "StartViewDataAndroid.h"
#elif Z_MACOS
#include "StartViewDataMacOS.h"
#elif Z_IOS
#include "StartViewDataiOS.h"
#else
#error >>>>> StartViewPlatformConfig header inclusion: Unsupported platform.
#endif

namespace zzz::core
{
#if Z_EDITOR || Z_WINDOWS
	using StartViewPlatformData = StartViewDataMSWin;
#elif Z_LINUX
	using StartViewPlatformData = StartViewDataLinux;
#elif Z_ANDROID
	using StartViewPlatformData = StartViewDataAndroid;
#elif Z_MACOS
	using StartViewPlatformData = StartViewDataMacOS;
#elif Z_IOS
	using StartViewPlatformData = StartViewDataiOS;
#else
#error >>>>> StartViewPlatformData alias: Unsupported platform.
#endif
}
