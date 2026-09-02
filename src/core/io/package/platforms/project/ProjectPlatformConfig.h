#pragma once

#include "core/utils/Defines.h"

#if defined(Z_EDITOR) || defined(Z_WINDOWS)
#include "ProjectPlatformDataMSWin.h"
#elif defined(Z_LINUX)
#include "ProjectPlatformDataLinux.h"
#elif defined(Z_ANDROID)
#include "ProjectPlatformDataAndroid.h"
#elif defined(Z_MACOS)
#include "ProjectPlatformDataMacOS.h"
#elif defined(Z_IOS)
#include "ProjectPlatformDataiOS.h"
#else
#error >>>>> ProjectPlatformConfig header inclusion: Unsupported platform.
#endif

namespace zzz::core
{
#if defined(Z_EDITOR) || defined(Z_WINDOWS)
	using ProjectPlatformData = ProjectPlatformDataMSWin;
#elif defined(Z_LINUX)
	using ProjectPlatformData = ProjectPlatformDataLinux;
#elif defined(Z_ANDROID)
	using ProjectPlatformData = ProjectPlatformDataAndroid;
#elif defined(Z_MACOS)
	using ProjectPlatformData = ProjectPlatformDataMacOS;
#elif defined(Z_IOS)
	using ProjectPlatformData = ProjectPlatformDataiOS;
#else
#error >>>>> ProjectPlatformData alias: Unsupported platform.
#endif
}
