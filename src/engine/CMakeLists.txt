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
    "private/core/Config/ConfigManager.h" "private/core/Config/ConfigManager.cpp"
    "private/core/IO/Path.h" "private/core/IO/Path.cpp"
    "private/core/Config/EngineConfig.h" "private/core/Config/EngineConfig.cpp"
    "private/core/Serialize/Serializer.h" "private/core/Serialize/Serializer.cpp"
    "private/core/Utils/version.h" "private/core/Utils/version.cpp" "headers/enums.h")

target_compile_features(engine_lib PUBLIC cxx_std_23)

if(ANDROID)
    find_package(game-activity REQUIRED CONFIG)
endif()

if(APPLE)
    target_sources(engine_lib PRIVATE "private/core/IO/platforms/Path_Apple.mm")
endif()

target_link_libraries(engine_lib
    PUBLIC
        logger_lib
    PRIVATE
        $<$<PLATFORM_ID:Android>:game-activity::game-activity_static>
)
