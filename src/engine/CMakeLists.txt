add_library(engine_lib)

target_precompile_headers(engine_lib PRIVATE
    pch/pch.h
)

target_include_directories(engine_lib
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
    PRIVATE
        pch
        ${CMAKE_CURRENT_SOURCE_DIR}/private/core
        ${CMAKE_CURRENT_SOURCE_DIR}/private/platforms
)

target_sources(engine_lib PRIVATE
    # Public
    engine.h
    engine.cpp
    header.h
    NativeAppData.h
    pch/pch.h
    pch/pch.cpp

    # Headers
    "headers/Apple.h"
    "headers/MSWin.h"
    "headers/Linux.h"
    "headers/Android.h"
    "headers/constants.h"
    "headers/enums.h"
    "headers/throwWrappers.h"

    # Core
    "private/platforms/config/ConfigManager.h" "private/platforms/config/ConfigManager.cpp"
    "private/platforms/config/EngineConfig.h" "private/platforms/config/EngineConfig.cpp"
    "private/platforms/config/PlatformConfig.h"
    "private/core/io/Path.h" "private/core/io/Path.cpp"
    "private/core/serialize/Serializer.h" "private/core/serialize/Serializer.cpp"
    "private/core/utils/version.h" "private/core/utils/version.cpp"
    "private/core/templates/Size2D.h"

    # Inputs
    "private/platforms/input/Input.h" "private/platforms/input/InputBase.h"

    # Platforms
    "private/platforms/Platform.h"
    "private/platforms/Platform_Common.cpp"

    # Main Loop
    "private/platforms/main_loop/MainLoop.h" "private/platforms/main_loop/MainLoop_Common.h" "private/platforms/main_loop/MainLoop_Common.cpp"

    # Native View / Window
    "private/platforms/window/Window.h" "private/platforms/window/Window_Common.h" "private/platforms/window/Window_Common.cpp"
    "private/core/view/View.h" "private/core/view/View.cpp"
    "private/core/view/ViewManager.h" "private/core/view/ViewManager.cpp"
    "private/core/utils/ScreenResolution.h"

    # Wayland (Linux)
    "private/core/specific/linux_wayland/xdg-shell-client-protocol.h"
    "private/core/specific/linux_wayland/xdg-shell-protocol.cpp"
  "private/core/utils/converters.h" "private/platforms/input/InputHelpers.h" "private/platforms/input/InputBase.cpp")

if(WIN32)
    target_sources(engine_lib PRIVATE 
        "private/platforms/Platform_Windows.cpp"
        "private/platforms/config/ConfigMSWin.h" "private/platforms/config/ConfigMSWin.cpp"
        "private/platforms/input/InputMSWindows.h" "private/platforms/input/InputMSWindows.cpp"
        "private/platforms/main_loop/MainLoop_MSWin.h" "private/platforms/main_loop/MainLoop_MSWin.cpp"
        "private/platforms/window/WinMSWindows.h" "private/platforms/window/WinMSWindows.cpp"
    )
endif()

if(UNIX AND NOT APPLE AND NOT ANDROID)
    target_sources(engine_lib PRIVATE 
        "private/platforms/Platform_Linux.cpp"
        "private/platforms/config/ConfigLinux.h" "private/platforms/config/ConfigLinux.cpp"
        "private/platforms/input/InputLinux.h" "private/platforms/input/InputLinux.cpp"
        "private/platforms/main_loop/MainLoop_Linux.h" "private/platforms/main_loop/MainLoop_Linux.cpp"
        "private/platforms/window/WinLinux.h" "private/platforms/window/WinLinux.cpp"
    )
endif()

if(ANDROID)
    target_sources(engine_lib PRIVATE 
        "private/platforms/Platform_Android.cpp"
        "private/platforms/config/ConfigAndroid.h" "private/platforms/config/ConfigAndroid.cpp"
        "private/platforms/input/InputAndroid.h" "private/platforms/input/InputAndroid.cpp"
        "private/platforms/main_loop/MainLoop_Android.h" "private/platforms/main_loop/MainLoop_Android.cpp"
        "private/platforms/window/WinAndroid.h" "private/platforms/window/WinAndroid.cpp"
    )
endif()

if(APPLE)
    target_sources(engine_lib PRIVATE
        # Core
        "private/core/io/apple/Path_Apple.mm"
    )

    if(IOS)
        target_sources(engine_lib PRIVATE
            "private/platforms/config/ConfigiOS.h" "private/platforms/config/ConfigiOS.mm"
            "private/platforms/input/InputiOS.h" "private/platforms/input/InputiOS.mm"
            "private/platforms/Platform_iOS.mm"
            "private/platforms/main_loop/MainLoop_iOS.h" "private/platforms/main_loop/MainLoop_iOS.mm"
            "private/platforms/window/WiniOS.h" "private/platforms/window/WiniOS.mm"
        )
    else()
        target_sources(engine_lib PRIVATE
            "private/platforms/config/ConfigMacOS.h" "private/platforms/config/ConfigMacOS.mm"
            "private/platforms/input/InputMacOS.h" "private/platforms/input/InputMacOS.mm"
            "private/platforms/Platform_MacOS.mm"
            "private/platforms/main_loop/MainLoop_MacOS.h" "private/platforms/main_loop/MainLoop_MacOS.mm"
            "private/platforms/window/WinMacOS.h" "private/platforms/window/WinMacOS.mm"
        )
    endif()
endif()

target_compile_features(engine_lib PUBLIC cxx_std_23)

if(UNIX AND NOT APPLE AND NOT ANDROID)
    target_link_libraries(engine_lib PRIVATE wayland-client)

    set_source_files_properties(
        "private/core/specific/linux_wayland/xdg-shell-protocol.cpp"
        PROPERTIES COMPILE_OPTIONS "-Wno-attributes"
    )
endif()

if(ANDROID)
    find_package(game-activity REQUIRED CONFIG)
endif()

target_link_libraries(engine_lib
    PUBLIC
        logger_lib
        common_lib
)
