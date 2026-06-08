#pragma once

#if defined(Z_ANDROID)
struct android_app;
#elif defined(Z_MACOS) || defined(Z_IOS)
// Apple specific forward declarations
#endif

namespace zzz::engine
{
#if defined(Z_ANDROID)
	using PlatformNativeData = android_app;
#elif defined(Z_MACOS) || defined(Z_IOS)
	using PlatformNativeData = void;
#else
	using PlatformNativeData = void;
#endif
}
