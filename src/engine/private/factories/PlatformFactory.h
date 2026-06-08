#pragma once

#include "../platforms/platforms/IPlatform.h"
#include "../platforms/platforms/PlatformMSWindows.h"
#include "../platforms/platforms/PlatformLinux.h"
#include "../platforms/platforms/PlatformAndroid.h"
#include "../platforms/platforms/PlatformMacOS.h"
#include "../platforms/platforms/PlatformiOS.h"

namespace zzz::engine
{
#if defined(Z_WINDOWS)
	using Platform = PlatformMSWindows;
#elif defined(Z_LINUX)
	using Platform = PlatformLinux;
#elif defined(Z_ANDROID)
	using Platform = PlatformAndroid;
#elif defined(Z_MACOS)
	using Platform = PlatformMacOS;
#elif defined(Z_IOS)
	using Platform = PlatformiOS;
#else
#error ">>>>> [Compile error]. This branch requires implementation for the current platform"
#endif

	class PlatformFactory final
	{
	public:
		std::shared_ptr<IPlatform> Create(std::string_view appName, std::shared_ptr<PlatformNativeData> platformData);
	};
}
