#pragma once

#if Z_EDITOR || Z_WINDOWS
#include "ProjectPlatformDataMSWin.h"
#elif Z_LINUX
#include "ProjectPlatformDataLinux.h"
#elif Z_ANDROID
#include "ProjectPlatformDataAndroid.h"
#elif Z_MACOS
#include "ProjectPlatformDataMacOS.h"
#elif Z_IOS
#include "ProjectPlatformDataiOS.h"
#else
#error >>>>> ProjectPlatformConfig header inclusion: Unsupported platform.
#endif

namespace zzz::core
{
#if Z_EDITOR || Z_WINDOWS
	using ProjectPlatformData = ProjectPlatformDataMSWin;
#elif Z_LINUX
	using ProjectPlatformData = ProjectPlatformDataLinux;
#elif Z_ANDROID
	using ProjectPlatformData = ProjectPlatformDataAndroid;
#elif Z_MACOS
	using ProjectPlatformData = ProjectPlatformDataMacOS;
#elif Z_IOS
	using ProjectPlatformData = ProjectPlatformDataiOS;
#else
#error >>>>> ProjectPlatformData alias: Unsupported platform.
#endif
}
