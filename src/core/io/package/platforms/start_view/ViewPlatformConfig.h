#pragma once

#include "core/utils/Defines.h"

#if defined(Z_EDITOR) || defined(Z_WINDOWS)
#include "ViewDataMSWin.h"
#elif defined(Z_LINUX)
#include "ViewDataLinux.h"
#elif defined(Z_ANDROID)
#include "ViewDataAndroid.h"
#elif defined(Z_MACOS)
#include "ViewDataMacOS.h"
#elif defined(Z_IOS)
#include "ViewDataiOS.h"
#else
#error >>>>> ViewPlatformConfig header inclusion: Unsupported platform.
#endif

namespace zzz::core
{
#if defined(Z_EDITOR) || defined(Z_WINDOWS)
	using ViewPlatformData = ViewDataMSWin;
#elif defined(Z_LINUX)
	using ViewPlatformData = ViewDataLinux;
#elif defined(Z_ANDROID)
	using ViewPlatformData = ViewDataAndroid;
#elif defined(Z_MACOS)
	using ViewPlatformData = ViewDataMacOS;
#elif defined(Z_IOS)
	using ViewPlatformData = ViewDataiOS;
#else
#error >>>>> ViewPlatformData alias: Unsupported platform.
#endif
}
