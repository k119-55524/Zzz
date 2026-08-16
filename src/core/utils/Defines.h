#pragma once

/* -------------------------------------------------------------

	Макросы, задаваемые в current.cmake утилитами BuildConfigurator и build_configurator_switch:
	- Z_FORCE_VULKAN      - принудительно использовать Vulkan на Windows.
	- Z_IDE_OUT_LOGS      - вывод логов в IDE в случае вывода логов

	Режимы сборки:
	- Z_EDITOR            - сборка движка для редактора

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
	- Z_DEVELOPMENT_BUILD - Оптимизированная сборка с выводом логов и проверками значений
	- Z_RELEASE_BUILD     - Release сборка (если определен NDEBUG)

   ------------------------------------------------------------- */

// Определение платформы
#if defined(_WIN32) && !defined(_WIN64)
#error >>>>> defines.h: 32-bit Windows is not supported.
#elif defined(_WIN64)
#define Z_WINDOWS 1

// Android
#elif defined(__ANDROID__) || defined(ANDROID)
#define Z_ANDROID 1

// Linux
#elif defined(__linux__) || defined(__linux)
#define Z_LINUX 1

// Apple (macOS / iOS)
#elif defined(__APPLE__)
#include <TargetConditionals.h>
#if TARGET_OS_IOS || TARGET_OS_TV
#define Z_IOS 1
#elif TARGET_OS_MAC
#define Z_MACOS 1
#else
#define Z_MACOS 1
#endif

// Неизвестная платформа
#else
#error >>>>> defines.h: Unknown or unsupported platform.
#endif

// Категории платформ
#if Z_WINDOWS || Z_LINUX || Z_MACOS
#define Z_DESKTOP 1
#endif
#if Z_ANDROID || Z_IOS
#define Z_MOBILE 1
#endif
#if Z_MACOS || Z_IOS
#define Z_APPLE 1
#endif

// Принудительное переопределение API через Z_FORCE_VULKAN
#if Z_FORCE_VULKAN && Z_WINDOWS
#undef Z_D3D12
#undef Z_VULKAN
#undef Z_METAL
#define Z_VULKAN 1
#endif

// Автоматический выбор графического API
#if !Z_D3D12 && !Z_VULKAN && !Z_METAL
#if Z_WINDOWS
#define Z_D3D12 1
#elif Z_ANDROID || Z_LINUX
#define Z_VULKAN 1
#elif Z_MACOS || Z_IOS
#define Z_METAL 1
#else
#error >>>>> defines.h: No suitable graphics API defined for this platform.
#endif
#endif // render api selection

#if defined(NDEBUG)
    #define Z_RELEASE_BUILD 1
#elif defined(_DEBUG) || defined(DEBUG)
    #define Z_DEBUG_BUILD 1
#endif

// Если компилируется Release-сборка — гарантированно выключаем все виды отладочных проверок и логов,
// даже если они случайно были переданы через настройки проекта или CMake.
#if Z_RELEASE_BUILD
    #undef Z_ADD_LOGGER
    #define Z_ADD_LOGGER 0
    #undef Z_IDE_OUT_LOGS
    #define Z_IDE_OUT_LOGS 0
#endif

// Вывод активных дефайнов
#if defined(Z_PRINT_DEFINES)
#pragma message(">>>>> ------- [ zdefines.h : active defines ] -------")
#ifdef Z_TARGET_NAME
#pragma message(">>>>> Target   : " Z_TARGET_NAME)
#endif
#if Z_WINDOWS
#pragma message(">>>>> Platform : Z_WINDOWS")
#endif
#if Z_LINUX
#pragma message(">>>>> Platform : Z_LINUX")
#endif
#if Z_MACOS
#pragma message(">>>>> Platform : Z_MACOS")
#endif
#if Z_IOS
#pragma message(">>>>> Platform : Z_IOS")
#endif
#if Z_ANDROID
#pragma message(">>>>> Platform : Z_ANDROID")
#endif
#if Z_DESKTOP
#pragma message(">>>>> Category : Z_DESKTOP")
#endif
#if Z_MOBILE
#pragma message(">>>>> Category : Z_MOBILE")
#endif
#if Z_APPLE
#pragma message(">>>>> Category : Z_APPLE")
#endif
#if Z_D3D12
#pragma message(">>>>> Graphics : Z_D3D12")
#endif
#if Z_VULKAN
#pragma message(">>>>> Graphics : Z_VULKAN")
#endif
#if Z_METAL
#pragma message(">>>>> Graphics : Z_METAL")
#endif
#if Z_DEBUG_BUILD
#pragma message(">>>>> Build    : Z_DEBUG_BUILD")
#endif
#if Z_DEVELOPMENT_BUILD
#pragma message(">>>>> Build    : Z_DEVELOPMENT_BUILD")
#endif
#if Z_RELEASE_BUILD
#pragma message(">>>>> Build    : Z_RELEASE_BUILD")
#endif
#if Z_ADD_LOGGER
#pragma message(">>>>> Logger   : Z_ADD_LOGGER")
#endif
#if Z_IDE_OUT_LOGS
#pragma message(">>>>> Logger   : Z_IDE_OUT_LOGS")
#endif
#if Z_EDITOR
#pragma message(">>>>> Mode     : Z_EDITOR")
#endif
#pragma message(">>>>> -----------------------------------------------")
#endif