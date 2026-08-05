#pragma once

#if Z_ANDROID
struct android_app;
#elif Z_LINUX
#include <cstdint>
struct wl_display;
struct wl_registry;
struct wl_compositor;
struct wl_shm;
struct xdg_wm_base;
struct wl_output;
#elif Z_EDITOR
struct HWND__;
typedef HWND__* HWND;
#endif

namespace zzz::core
{
#if Z_ANDROID
	using NativeAppData = android_app;
#elif Z_LINUX
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
#elif Z_EDITOR
	struct NativeAppData
	{
		HWND hwnd = nullptr;
	};
#else
	using NativeAppData = void;
#endif
}
