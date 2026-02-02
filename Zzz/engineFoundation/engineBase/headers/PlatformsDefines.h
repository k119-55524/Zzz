#pragma once

/* -------------------------------------------------------------

	Кастомные макросы которые используются в проекте(задаются в свойствах проекта):
		1. `ZTEST` - используется для включения тестовых проверок и сообщений.
		2. `ZLOG` - используется для логирования сообщений.
		3. `ZVULKAN` - принудительно использовать апи VULKAN в Windows.

   ------------------------------------------------------------- */

	// Сначала определяем платформу (если ещё не определена)
#if !defined(ZPLATFORM_MSWINDOWS) && !defined(ZPLATFORM_LINUX) && !defined(ZPLATFORM_MACOS) && \
	!defined(ZPLATFORM_ANDROID) && !defined(ZPLATFORM_XBOX) && !defined(ZPLATFORM_PLAYSTATION) && \
	!defined(ZPLATFORM_NINTENDO_SWITCH)

	// -------------------------------------------------------------
	// XBOX SERIES X|S (GDK) — НОВОЕ ПОКОЛЕНИЕ
	// -------------------------------------------------------------
#if defined(_GAMING_XBOX) || (defined(WINAPI_FAMILY) && WINAPI_FAMILY == WINAPI_FAMILY_GAMES)
#define ZPLATFORM_XBOX
#ifdef ZLOG
#pragma message(">>>>> Target: Xbox Series X|S (GDK)")
#endif

// -------------------------------------------------------------
// PLAYSTATION 5 (Prospero) — НОВОЕ ПОКОЛЕНИЕ
// -------------------------------------------------------------
#elif defined(__PROSPERO__)
#define ZPLATFORM_PLAYSTATION
#ifdef ZLOG
#pragma message(">>>>> Target: PlayStation 5 (Prospero)")
#endif

// -------------------------------------------------------------
// NINTENDO SWITCH
// -------------------------------------------------------------
#elif defined(__NX__) || defined(__SWITCH__)
#define ZPLATFORM_NINTENDO_SWITCH
#ifdef ZLOG
#pragma message(">>>>> Target: Nintendo Switch")
#endif

// -------------------------------------------------------------
// WINDOWS DESKTOP
// -------------------------------------------------------------
#elif defined(_WIN64)
	// Явно исключаем Xbox One (Durango/XDK)
#if defined(_DURANGO) || defined(_XBOX_ONE)
#error ">>>>> Xbox One is not supported. Only Xbox Series X|S (GDK) is allowed."
#else
#define ZPLATFORM_MSWINDOWS
#ifdef ZLOG
#pragma message(">>>>> Target: Windows Desktop")
#endif
#endif

// -------------------------------------------------------------
// ANDROID
// -------------------------------------------------------------
#elif defined(__ANDROID__) || defined(ANDROID)
#define ZPLATFORM_ANDROID
#ifdef ZLOG
#pragma message(">>>>> Target: Android")
#endif

// -------------------------------------------------------------
// LINUX
// -------------------------------------------------------------
#elif defined(__linux__) || defined(__linux)
#define ZPLATFORM_LINUX
#ifdef ZLOG
#pragma message(">>>>> Target: Linux")
#endif

// -------------------------------------------------------------
// APPLE (macOS / iOS)
// -------------------------------------------------------------
#elif defined(__APPLE__)
#include <TargetConditionals.h>
#if TARGET_OS_IOS || TARGET_OS_TV
#define ZPLATFORM_IOS
#ifdef ZLOG
#pragma message(">>>>> Target: iOS/tvOS")
#endif
#elif TARGET_OS_MAC
#define ZPLATFORM_MACOS
#ifdef ZLOG
#pragma message(">>>>> Target: macOS")
#endif
#else
#define ZPLATFORM_MACOS
#endif

// -------------------------------------------------------------
// PLAYSTATION 4 — ЯВНО ЗАПРЕЩАЕМ
// -------------------------------------------------------------
#elif defined(__ORBIS__)
#error "PlayStation 4 (Orbis) is not supported. Only PlayStation 5 (Prospero) is allowed."

// -------------------------------------------------------------
// НЕИЗВЕСТНАЯ ПЛАТФОРМА
// -------------------------------------------------------------
#else
#error "Unknown or unsupported platform! Define PLATFORM_* manually if needed."
#endif

#endif

// -------------------------------------------------------------
// ПРИНУДИТЕЛЬНОЕ ПЕРЕОПРЕДЕЛЕНИЕ ЧЕРЕЗ ZVULKAN (ТОЛЬКО НА WINDOWS!)
// -------------------------------------------------------------
#if defined(ZVULKAN) && defined(ZPLATFORM_MSWINDOWS)
#undef ZRENDER_API_D3D12
#undef ZRENDER_API_VULKAN
#undef ZRENDER_API_VULKAN
#define ZRENDER_API_VULKAN
#ifdef ZLOG
#pragma message(">>>>> ZVULKAN enabled on Windows: Forcing Vulkan API.")
#endif
#endif

// -------------------------------------------------------------
// АВТОМАТИЧЕСКИЙ ВЫБОР API
// -------------------------------------------------------------
#if !defined(ZRENDER_API_D3D12) && !defined(ZRENDER_API_VULKAN) && !defined(ZRENDER_API_METAL)
#if defined(ZPLATFORM_MSWINDOWS) || defined(ZPLATFORM_XBOX)
#define ZRENDER_API_D3D12
#elif defined(ZPLATFORM_ANDROID) || defined(ZPLATFORM_LINUX) || defined(ZPLATFORM_PLAYSTATION) || defined(ZPLATFORM_NINTENDO_SWITCH)
#define ZRENDER_API_VULKAN
#elif defined(ZPLATFORM_MACOS) || defined(ZPLATFORM_IOS)
#define ZRENDER_API_METAL
#else
#error "No suitable graphics API defined for this platform!"
#endif
#endif