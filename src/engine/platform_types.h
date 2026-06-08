#pragma once

#if defined(Z_ANDROID)
struct android_app;
#endif

namespace zzz::engine
{
#if defined(Z_ANDROID)
	using PlatformNativeData = android_app;
#else
	using PlatformNativeData = void;
#endif
}
