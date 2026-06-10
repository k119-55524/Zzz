#pragma once

#if defined(Z_ANDROID)
struct android_app;
#endif

namespace zzz::engine
{
#if defined(Z_ANDROID)
	using NativeAppData = android_app;
#else
	using NativeAppData = void;
#endif
}
