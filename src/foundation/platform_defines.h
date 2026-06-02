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
#if !defined(Z_WINDOWS) && \
    !defined(Z_LINUX)   && \
    !defined(Z_MACOS)   && \
    !defined(Z_IOS)     && \
    !defined(Z_ANDROID)

// -------------------------------------------------------------
// Windows x64
// -------------------------------------------------------------
#if defined(_WIN32) && !defined(_WIN64)
#error "32-bit Windows is not supported."
#elif defined(_WIN64)
#define Z_WINDOWS
#ifdef ZLOG
#pragma message(">>>>> Target: Windows x64")
#endif

// -------------------------------------------------------------
// Android
// -------------------------------------------------------------
#elif defined(__ANDROID__) || defined(ANDROID)
#define Z_ANDROID
#ifdef ZLOG
#pragma message(">>>>> Target: Android")
#endif

// -------------------------------------------------------------
// Linux
// -------------------------------------------------------------
#elif defined(__linux__) || defined(__linux)
#define Z_LINUX
#ifdef ZLOG
#pragma message(">>>>> Target: Linux")
#endif

// -------------------------------------------------------------
// Apple (macOS / iOS)
// -------------------------------------------------------------
#elif defined(__APPLE__)
#include <TargetConditionals.h>
#if TARGET_OS_IOS || TARGET_OS_TV
#define Z_IOS
#ifdef ZLOG
#pragma message(">>>>> Target: iOS/tvOS")
#endif
#elif TARGET_OS_MAC
#define Z_MACOS
#ifdef ZLOG
#pragma message(">>>>> Target: macOS")
#endif
#else
#define Z_MACOS
#endif

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
#if defined(Z_WINDOWS) || \
    defined(Z_LINUX)   || \
    defined(Z_MACOS)
#define Z_DESKTOP
#endif
#if defined(Z_ANDROID) || \
    defined(Z_IOS)
#define Z_MOBILE
#endif

// -------------------------------------------------------------
// Принудительное переопределение API через ZVULKAN
// -------------------------------------------------------------
#if defined(ZVULKAN) && defined(Z_WINDOWS)
#undef Z_D3D12
#undef Z_VULKAN
#undef Z_METAL
#define Z_VULKAN
#ifdef ZLOG
#pragma message(">>>>> ZVULKAN enabled on Windows: forcing Vulkan API.")
#endif
#endif

// -------------------------------------------------------------
// Автоматический выбор графического API
// -------------------------------------------------------------
#if !defined(Z_D3D12) && \
    !defined(Z_VULKAN) && \
    !defined(Z_METAL)
#if defined(Z_WINDOWS)
#define Z_D3D12
#elif defined(Z_ANDROID) || \
      defined(Z_LINUX)
#define Z_VULKAN
#elif defined(Z_MACOS) || \
      defined(Z_IOS)
#define Z_METAL
#else
#error "No suitable graphics API defined for this platform."
#endif
#endif // render api selection