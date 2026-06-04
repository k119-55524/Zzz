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
)

target_sources(engine_lib PRIVATE
    engine.h
    engine.cpp
    header.h
    "headers/Apple.h"
    "headers/MSWin.h"
    "headers/Linux.h"
    "headers/Android.h"
    "headers/constants.h"
    "private/core/config/ConfigManager.h" "private/core/config/ConfigManager.cpp"
    "private/core/io/Path.h" "private/core/io/Path.cpp"
    "private/core/config/EngineConfig.h" "private/core/config/EngineConfig.cpp"
    "private/core/serialize/Serializer.h" "private/core/serialize/Serializer.cpp"
    "private/core/utils/version.h" "private/core/utils/version.cpp"
    "headers/enums.h"
    "headers/throwWrappers.h"
    "pch/pch.cpp"
    "private/platforms/native_view/window/IWindow.h" "private/platforms/native_view/window/IWindow.cpp"
    "private/platforms/native_view/window/MSWin_Window.h" "private/platforms/native_view/window/MSWin_Window.cpp"
    "private/factories/PlatformFactory.h" "private/factories/PlatformFactory.cpp"
    "private/platforms/native_view/NativeView.h" "private/platforms/native_view/NativeView.cpp"
    "private/core/config/platforms/ConfigMSWin.h" "private/core/config/platforms/ConfigMSWin.cpp"
    "private/core/templates/Size2D.h"  "private/platforms/native_view/ScreenResolution.h"  "private/core/templates/Event.h" "private/platforms/main_loop/IMainLoop.h" "private/platforms/main_loop/MainLoop_MSWin.h" "private/platforms/main_loop/MainLoop_MSWin.cpp")

target_compile_features(engine_lib PUBLIC cxx_std_23)

if(ANDROID)
    find_package(game-activity REQUIRED CONFIG)
endif()

if(APPLE)
    target_sources(engine_lib PRIVATE "private/core/io/platforms/Path_Apple.mm")
endif()

target_link_libraries(engine_lib
    PUBLIC
        logger_lib
    PRIVATE
        $<$<PLATFORM_ID:Android>:game-activity::game-activity_static>
)
