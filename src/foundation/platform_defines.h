#pragma once

/* -------------------------------------------------------------

	Кастомные макросы, используемые в проекте
	(задаются в свойствах проекта):

		ZTEST   - включает тестовые проверки и сообщения.
		ZLOG    - включает диагностические сообщения сборки.
		ZVULKAN - принудительно использовать Vulkan на Windows.

   ------------------------------------------------------------- */

   // -------------------------------------------------------------
   // Определение платформы
   // -------------------------------------------------------------
#if !defined(ZPLATFORM_MSWINDOWS)      && \
    !defined(ZPLATFORM_LINUX)          && \
    !defined(ZPLATFORM_MACOS)          && \
    !defined(ZPLATFORM_IOS)            && \
    !defined(ZPLATFORM_ANDROID)        && \
    !defined(ZPLATFORM_XBOX)           && \
    !defined(ZPLATFORM_PLAYSTATION)    && \
    !defined(ZPLATFORM_NINTENDO_SWITCH)

// -------------------------------------------------------------
// Xbox Series X|S (GDK)
// -------------------------------------------------------------
#if defined(_GAMING_XBOX) || (defined(WINAPI_FAMILY) && WINAPI_FAMILY == WINAPI_FAMILY_GAMES)

#define ZPLATFORM_XBOX

#ifdef ZLOG
#pragma message(">>>>> Target: Xbox Series X|S (GDK)")
#endif

// -------------------------------------------------------------
// PlayStation 5 (Prospero)
// -------------------------------------------------------------
#elif defined(__PROSPERO__)

#define ZPLATFORM_PLAYSTATION

#ifdef ZLOG
#pragma message(">>>>> Target: PlayStation 5 (Prospero)")
#endif

// -------------------------------------------------------------
// Nintendo Switch
// -------------------------------------------------------------
#elif defined(__NX__) || defined(__SWITCH__)

#define ZPLATFORM_NINTENDO_SWITCH

#ifdef ZLOG
#pragma message(">>>>> Target: Nintendo Switch")
#endif

// -------------------------------------------------------------
// Windows x64
// -------------------------------------------------------------
#elif defined(_WIN32) && !defined(_WIN64)

#error "32-bit Windows is not supported."

#elif defined(_WIN64)

#if defined(_DURANGO) || defined(_XBOX_ONE)
#error "Xbox One is not supported. Only Xbox Series X|S (GDK) is allowed."
#endif

#define ZPLATFORM_MSWINDOWS

#ifdef ZLOG
#pragma message(">>>>> Target: Windows x64")
#endif

// -------------------------------------------------------------
// Android
// -------------------------------------------------------------
#elif defined(__ANDROID__) || defined(ANDROID)

#define ZPLATFORM_ANDROID

#ifdef ZLOG
#pragma message(">>>>> Target: Android")
#endif

// -------------------------------------------------------------
// Linux
// -------------------------------------------------------------
#elif defined(__linux__) || defined(__linux)

#define ZPLATFORM_LINUX

#ifdef ZLOG
#pragma message(">>>>> Target: Linux")
#endif

// -------------------------------------------------------------
// Apple (macOS / iOS)
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
// PlayStation 4 (Orbis)
// -------------------------------------------------------------
#elif defined(__ORBIS__)

#error "PlayStation 4 (Orbis) is not supported. Only PlayStation 5 (Prospero) is allowed."

// -------------------------------------------------------------
// Неизвестная платформа
// -------------------------------------------------------------
#else

#error "Unknown or unsupported platform."

#endif

#endif // platform detection

// -------------------------------------------------------------
// Категории платформ
// -------------------------------------------------------------
#if defined(ZPLATFORM_MSWINDOWS) || \
    defined(ZPLATFORM_LINUX)     || \
    defined(ZPLATFORM_MACOS)

#define ZPLATFORM_DESKTOP

#endif

#if defined(ZPLATFORM_ANDROID) || \
    defined(ZPLATFORM_IOS)

#define ZPLATFORM_MOBILE

#endif

// -------------------------------------------------------------
// Принудительное переопределение API через ZVULKAN
// -------------------------------------------------------------
#if defined(ZVULKAN) && defined(ZPLATFORM_MSWINDOWS)

#undef ZRENDER_API_D3D12
#undef ZRENDER_API_VULKAN
#undef ZRENDER_API_METAL

#define ZRENDER_API_VULKAN

#ifdef ZLOG
#pragma message(">>>>> ZVULKAN enabled on Windows: forcing Vulkan API.")
#endif

#endif

// -------------------------------------------------------------
// Автоматический выбор графического API
// -------------------------------------------------------------
#if !defined(ZRENDER_API_D3D12) && \
    !defined(ZRENDER_API_VULKAN) && \
    !defined(ZRENDER_API_METAL)

#if defined(ZPLATFORM_MSWINDOWS) || defined(ZPLATFORM_XBOX)

#define ZRENDER_API_D3D12

#elif defined(ZPLATFORM_ANDROID)         || \
      defined(ZPLATFORM_LINUX)           || \
      defined(ZPLATFORM_PLAYSTATION)     || \
      defined(ZPLATFORM_NINTENDO_SWITCH)

#define ZRENDER_API_VULKAN

#elif defined(ZPLATFORM_MACOS) || defined(ZPLATFORM_IOS)

#define ZRENDER_API_METAL

#else

#error "No suitable graphics API defined for this platform."

#endif

#endif // render api selection