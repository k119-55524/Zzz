#pragma once

/* -------------------------------------------------------------

    Кастомные макросы которые используются в проекте(задаются в свойствах проекта):
        1. `ZTEST` - используется для включения тестовых проверок и сообщений.
        2. `ZLOG` - используется для логирования сообщений.
        3. `ZVULKAN` - принудительно использовать апи VULKAN в Windows.

   ------------------------------------------------------------- */

   // Сначала определяем платформу (если ещё не определена)
#if !defined(PLATFORM_WINDOWS) && !defined(PLATFORM_LINUX) && !defined(PLATFORM_MACOS) && \
    !defined(PLATFORM_ANDROID) && !defined(PLATFORM_XBOX) && !defined(PLATFORM_PLAYSTATION) && \
    !defined(PLATFORM_SWITCH)

    // -------------------------------------------------------------
    // XBOX SERIES X|S (GDK) — НОВОЕ ПОКОЛЕНИЕ
    // -------------------------------------------------------------
    #if defined(_GAMING_XBOX) || (defined(WINAPI_FAMILY) && WINAPI_FAMILY == WINAPI_FAMILY_GAMES)
        #define PLATFORM_XBOX
        #ifdef ZLOG
            #pragma message(">>>>> Target: Xbox Series X|S (GDK)")
    #endif

    // -------------------------------------------------------------
    // PLAYSTATION 5 (Prospero) — НОВОЕ ПОКОЛЕНИЕ
    // -------------------------------------------------------------
    #elif defined(__PROSPERO__)
        #define PLATFORM_PLAYSTATION
        #ifdef ZLOG
            #pragma message(">>>>> Target: PlayStation 5 (Prospero)")
    #endif

    // -------------------------------------------------------------
    // NINTENDO SWITCH
    // -------------------------------------------------------------
    #elif defined(__NX__) || defined(__SWITCH__)
        #define PLATFORM_SWITCH
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
            #define PLATFORM_WINDOWS
        #ifdef ZLOG
            #pragma message(">>>>> Target: Windows Desktop")
        #endif
    #endif

    // -------------------------------------------------------------
    // ANDROID
    // -------------------------------------------------------------
    #elif defined(__ANDROID__) || defined(ANDROID)
        #define PLATFORM_ANDROID
        #ifdef ZLOG
            #pragma message(">>>>> Target: Android")
    #endif

    // -------------------------------------------------------------
    // LINUX
    // -------------------------------------------------------------
    #elif defined(__linux__) || defined(__linux)
        #define PLATFORM_LINUX
        #ifdef ZLOG
            #pragma message(">>>>> Target: Linux")
    #endif

    // -------------------------------------------------------------
    // APPLE (macOS / iOS)
    // -------------------------------------------------------------
    #elif defined(__APPLE__)
        #include <TargetConditionals.h>
        #if TARGET_OS_IOS || TARGET_OS_TV
            #define PLATFORM_IOS
            #ifdef ZLOG
                #pragma message(">>>>> Target: iOS/tvOS")
            #endif
        #elif TARGET_OS_MAC
            #define PLATFORM_MACOS
            #ifdef ZLOG
                #pragma message(">>>>> Target: macOS")
            #endif
        #else
            #define PLATFORM_MACOS
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
#if defined(ZVULKAN) && defined(PLATFORM_WINDOWS)
    #undef RENDER_API_D3D12
    #undef RENDER_API_VULKAN
    #undef RENDER_API_METAL
    #define RENDER_API_VULKAN
    #ifdef ZLOG
        #pragma message(">>>>> ZVULKAN enabled on Windows: Forcing Vulkan API.")
    #endif
#endif

// -------------------------------------------------------------
// АВТОМАТИЧЕСКИЙ ВЫБОР API
// -------------------------------------------------------------
#if !defined(RENDER_API_D3D12) && !defined(RENDER_API_VULKAN) && !defined(RENDER_API_METAL)
    #if defined(PLATFORM_WINDOWS) || defined(PLATFORM_XBOX)
        #define RENDER_API_D3D12
    #elif defined(PLATFORM_ANDROID) || defined(PLATFORM_LINUX) || defined(PLATFORM_PLAYSTATION) || defined(PLATFORM_SWITCH)
        #define RENDER_API_VULKAN
    #elif defined(PLATFORM_MACOS) || defined(PLATFORM_IOS)
        #define RENDER_API_METAL
    #else
        #error "No suitable graphics API defined for this platform!"
    #endif
#endif