add_library(engine_lib)

target_precompile_headers(engine_lib PRIVATE
    pch/pch.h
)

target_include_directories(engine_lib
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/../foundation
        ${CMAKE_CURRENT_SOURCE_DIR}
    PRIVATE
        pch
        ${CMAKE_CURRENT_SOURCE_DIR}/private/core
        ${CMAKE_CURRENT_SOURCE_DIR}/private/platforms
        ${CMAKE_CURRENT_SOURCE_DIR}/private/factories
        ${CMAKE_CURRENT_SOURCE_DIR}/private/inputs
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
    "private/core/config/ConfigManager.h"           "private/core/config/ConfigManager.cpp"
    "private/core/config/EngineConfig.h"             "private/core/config/EngineConfig.cpp"
    "private/core/config/platforms/PlatformConfig.h"
    "private/core/io/Path.h"                         "private/core/io/Path.cpp"
    "private/core/serialize/Serializer.h"            "private/core/serialize/Serializer.cpp"
    "private/core/utils/version.h"                   "private/core/utils/version.cpp"
    "private/core/templates/Size2D.h"
    "private/core/templates/Event.h"

    # Inputs
    "private/platforms/input/Input.h" "private/platforms/input/Input_Common.h"

    # Platforms
    "private/platforms/Platform.h"
    "private/platforms/Platform_Common.cpp"

    # Main Loop
    "private/platforms/main_loop/MainLoop.h" "private/platforms/main_loop/MainLoop_Common.h" "private/platforms/main_loop/MainLoop_Common.cpp"

    # Native View / Window
    "private/platforms/window/Window.h" "private/platforms/window/Window_Common.h" "private/platforms/window/Window_Common.cpp"
    "private/core/view/View.h"            "private/core/view/View.cpp"
    "private/core/view/ViewManager.h" "private/core/view/ViewManager.cpp"
    "private/core/utils/ScreenResolution.h"

    # Wayland (Linux)
    "private/platforms/specific/Wayland/xdg-shell-client-protocol.h"
    "private/platforms/specific/Wayland/xdg-shell-protocol.cpp"
)

if(WIN32)
    target_sources(engine_lib PRIVATE 
        "private/platforms/Platform_Windows.cpp"
        "private/core/config/platforms/ConfigMSWin.h" "private/core/config/platforms/ConfigMSWin.cpp"
        "private/platforms/input/platforms/InputMSWindows.h" "private/platforms/input/platforms/InputMSWindows.cpp"
        "private/platforms/main_loop/MainLoop_MSWin.h" "private/platforms/main_loop/MainLoop_MSWin.cpp"
        "private/platforms/window/WinMSWindows.h" "private/platforms/window/WinMSWindows.cpp"
    )
endif()

if(UNIX AND NOT APPLE AND NOT ANDROID)
    target_sources(engine_lib PRIVATE 
        "private/platforms/Platform_Linux.cpp"
        "private/core/config/platforms/ConfigLinux.h" "private/core/config/platforms/ConfigLinux.cpp"
        "private/platforms/input/platforms/InputLinux.h" "private/platforms/input/platforms/InputLinux.cpp"
        "private/platforms/main_loop/MainLoop_Linux.h" "private/platforms/main_loop/MainLoop_Linux.cpp"
        "private/platforms/window/WinLinux.h" "private/platforms/window/WinLinux.cpp"
    )
endif()

if(ANDROID)
    target_sources(engine_lib PRIVATE 
        "private/platforms/Platform_Android.cpp"
        "private/core/config/platforms/ConfigAndroid.h" "private/core/config/platforms/ConfigAndroid.cpp"
        "private/platforms/input/platforms/InputAndroid.h" "private/platforms/input/platforms/InputAndroid.cpp"
        "private/platforms/main_loop/MainLoop_Android.h" "private/platforms/main_loop/MainLoop_Android.cpp"
        "private/platforms/window/WinAndroid.h" "private/platforms/window/WinAndroid.cpp"
    )
endif()

if(APPLE)
    target_sources(engine_lib PRIVATE
        # Core
        "private/core/io/platforms/Path_Apple.mm"
        "private/core/config/platforms/ConfigMacOS.h"   "private/core/config/platforms/ConfigMacOS.mm"
        "private/core/config/platforms/ConfigiOS.h"     "private/core/config/platforms/ConfigiOS.mm"

        # Inputs
    "private/platforms/input/Input.h" "private/platforms/input/Input_Common.h"
        "private/platforms/input/platforms/InputMacOS.h"         "private/platforms/input/platforms/InputMacOS.mm"
        "private/platforms/input/platforms/InputiOS.h"           "private/platforms/input/platforms/InputiOS.mm"

        # Platforms
        "private/platforms/Platform_MacOS.mm"
        "private/platforms/Platform_iOS.mm"

        # Main Loop
    "private/platforms/main_loop/MainLoop.h" "private/platforms/main_loop/MainLoop_Common.h" "private/platforms/main_loop/MainLoop_Common.cpp"
        "private/platforms/main_loop/MainLoop_MacOS.h"  "private/platforms/main_loop/MainLoop_MacOS.mm"
        "private/platforms/main_loop/MainLoop_iOS.h"    "private/platforms/main_loop/MainLoop_iOS.mm"

        # Native View / Window
    "private/platforms/window/Window.h" "private/platforms/window/Window_Common.h" "private/platforms/window/Window_Common.cpp"
        "private/platforms/window/WinMacOS.h"   "private/platforms/window/WinMacOS.mm"
        "private/platforms/window/WiniOS.h"     "private/platforms/window/WiniOS.mm"
    )
endif()

target_compile_features(engine_lib PUBLIC cxx_std_23)

if(UNIX AND NOT APPLE AND NOT ANDROID)
    target_link_libraries(engine_lib PRIVATE wayland-client)

    set_source_files_properties(
        "private/platforms/specific/Wayland/xdg-shell-protocol.cpp"
        PROPERTIES COMPILE_OPTIONS "-Wno-attributes"
    )
endif()

if(ANDROID)
    find_package(game-activity REQUIRED CONFIG)
endif()

target_link_libraries(engine_lib
    PUBLIC
        logger_lib
    PRIVATE
        $<$<PLATFORM_ID:Android>:game-activity::game-activity_static>
)
