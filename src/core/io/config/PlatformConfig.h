#pragma once

#if Z_EDITOR
#include "platforms/ConfigEditor.h"
#elif Z_WINDOWS
#include "platforms/ConfigMSWin.h"
#elif Z_LINUX
#include "platforms/ConfigLinux.h"
#elif Z_ANDROID
#include "platforms/ConfigAndroid.h"
#elif Z_MACOS
#include "platforms/ConfigMacOS.h"
#elif Z_IOS
#include "platforms/ConfigiOS.h"
#else
#error >>>>> PlatformConfig header inclusion: Unsupported platform.
#endif

namespace zzz::core
{
#if Z_EDITOR
	using PlatformConfig = ConfigEditor;
#elif Z_WINDOWS
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
#error >>>>> PlatformConfig alias: Unsupported platform.
#endif
}

