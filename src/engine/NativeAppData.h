#pragma once

#if defined(Z_ANDROID)
struct android_app;
#elif defined(Z_LINUX)
#include <stdint.h>
struct wl_display;
struct wl_registry;
struct wl_compositor;
struct wl_shm;
struct xdg_wm_base;
struct wl_output;
#endif

namespace zzz::engine
{
#if defined(Z_ANDROID)
	using NativeAppData = android_app;
#elif defined(Z_LINUX)
	struct NativeAppData
	{
		wl_display* display = nullptr;
		wl_registry* registry = nullptr;
		wl_compositor* compositor = nullptr;
		wl_shm* shm = nullptr;
		xdg_wm_base* xdgWmBase = nullptr;
		wl_output* output = nullptr;
		int32_t scaleFactor = 1;
	};
#else
	using NativeAppData = void;
#endif
}