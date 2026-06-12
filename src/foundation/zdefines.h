#pragma once

/* -------------------------------------------------------------

	Макросы, задаваемые в current.cmake утилитами BuildConfigurator и build_configurator_switch:
	- Z_FORCE_VULKAN  - принудительно использовать Vulkan на Windows.
	- Z_IDE_OUT_LOGS - вывод логов в IDE в случае вывода логов

	Макросы платформы (определяются автоматически):
	- Z_WINDOWS - Windows x64
	- Z_LINUX   - Linux (только десктоп)
	- Z_MACOS   - macOS
	- Z_IOS     - iOS / tvOS
	- Z_ANDROID - Android

	Категории платформ:
	- Z_DESKTOP - Windows, Linux, macOS
	- Z_MOBILE  - Android, iOS
	- Z_APPLE   - macOS, iOS

	Графические API (определяются автоматически):
	- Z_D3D12  - Direct3D 12 (Z_WINDOWS)
	- Z_VULKAN - Vulkan      (Z_ANDROID, Z_LINUX)
	- Z_METAL  - Metal       (Z_MACOS, Z_IOS)

	Типы сборки (определяются автоматически или передаются из CMake):
	- Z_DEBUG_BUILD       - Debug сборка (по умолчанию, если нет NDEBUG)
	- Z_DEVELOPMENT_BUILD - Development сборка (оптимизированная с дебаг-фичами, задается извне)
	- Z_RELEASE_BUILD     - Release сборка (если определен NDEBUG)

   ------------------------------------------------------------- */

// Определение платформы
#if !defined(Z_WINDOWS) && \
    !defined(Z_LINUX)   && \
    !defined(Z_MACOS)   && \
    !defined(Z_IOS)     && \
    !defined(Z_ANDROID)

// Windows x64
#if defined(_WIN32) && !defined(_WIN64)
#error >>>>> 32-bit Windows is not supported.
#elif defined(_WIN64)
#define Z_WINDOWS

// Android
#elif defined(__ANDROID__) || defined(ANDROID)
#define Z_ANDROID

// Linux
#elif defined(__linux__) || defined(__linux)
#define Z_LINUX

// Apple (macOS / iOS)
#elif defined(__APPLE__)
#include <TargetConditionals.h>
#if TARGET_OS_IOS || TARGET_OS_TV
#define Z_IOS
#elif TARGET_OS_MAC
#define Z_MACOS
#else
#define Z_MACOS
#endif

// Неизвестная платформа
#else
#error >>>>> Unknown or unsupported platform.
#endif
#endif // platform detection

// Категории платформ
#if defined(Z_WINDOWS) || \
    defined(Z_LINUX)   || \
    defined(Z_MACOS)
#define Z_DESKTOP
#endif
#if defined(Z_ANDROID) || \
    defined(Z_IOS)
#define Z_MOBILE
#endif
#if defined(Z_MACOS) || \
    defined(Z_IOS)
#define Z_APPLE
#endif

// Принудительное переопределение API через Z_FORCE_VULKAN
#if defined(Z_FORCE_VULKAN) && Z_FORCE_VULKAN && defined(Z_WINDOWS)
#undef Z_D3D12
#undef Z_VULKAN
#undef Z_METAL
#define Z_VULKAN
#endif

// Автоматический выбор графического API
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
#error >>>>> No suitable graphics API defined for this platform.
#endif
#endif // render api selection

// Автоматическое определение типа сборки (Debug / Release), 
// если снаружи (через CMake) не включен Z_DEVELOPMENT_BUILD.
#if !defined(Z_DEBUG_BUILD) && !defined(Z_RELEASE_BUILD) && (!defined(Z_DEVELOPMENT_BUILD) || Z_DEVELOPMENT_BUILD == 0)
    #if defined(_DEBUG) || defined(DEBUG) || !defined(NDEBUG)
        #define Z_DEBUG_BUILD 1
    #else
        #define Z_RELEASE_BUILD 1
    #endif
#endif

// Вывод активных дефайнов
#pragma message(">>>>> ------- [ zdefines.h : active defines ] -------")
#ifdef Z_WINDOWS
#pragma message(">>>>> Platform : Z_WINDOWS")
#endif
#ifdef Z_LINUX
#pragma message(">>>>> Platform : Z_LINUX")
#endif
#ifdef Z_MACOS
#pragma message(">>>>> Platform : Z_MACOS")
#endif
#ifdef Z_IOS
#pragma message(">>>>> Platform : Z_IOS")
#endif
#ifdef Z_ANDROID
#pragma message(">>>>> Platform : Z_ANDROID")
#endif
#ifdef Z_DESKTOP
#pragma message(">>>>> Category : Z_DESKTOP")
#endif
#ifdef Z_MOBILE
#pragma message(">>>>> Category : Z_MOBILE")
#endif
#ifdef Z_APPLE
#pragma message(">>>>> Category : Z_APPLE")
#endif
#ifdef Z_D3D12
#pragma message(">>>>> Graphics : Z_D3D12")
#endif
#ifdef Z_VULKAN
#pragma message(">>>>> Graphics : Z_VULKAN")
#endif
#ifdef Z_METAL
#pragma message(">>>>> Graphics : Z_METAL")
#endif
#if defined(Z_DEBUG_BUILD) && Z_DEBUG_BUILD
#pragma message(">>>>> Build    : Z_DEBUG_BUILD")
#endif
#if defined(Z_DEVELOPMENT_BUILD) && Z_DEVELOPMENT_BUILD
#pragma message(">>>>> Build    : Z_DEVELOPMENT_BUILD")
#endif
#if defined(Z_RELEASE_BUILD) && Z_RELEASE_BUILD
#pragma message(">>>>> Build    : Z_RELEASE_BUILD")
#endif
#pragma message(">>>>> -----------------------------------------------")
