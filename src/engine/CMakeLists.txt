add_library(engine_lib)

target_precompile_headers(engine_lib PRIVATE
    pch/pch.h
)

target_include_directories(engine_lib PUBLIC
    pch
    ${CMAKE_CURRENT_SOURCE_DIR}/../foundation
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_sources(engine_lib PRIVATE
    engine.h
    engine.cpp
    header.h
    "headers/Apple.h"
    "headers/MSWin.h"
    "headers/Linux.h"
    "private/core/Config/ConfigManager.h" "private/core/Config/ConfigManager.cpp"
    "private/core/IO/Path.h" "private/core/IO/Path.cpp" "private/core/Config/EngineConfig.h" "private/core/Config/EngineConfig.cpp")

target_compile_features(engine_lib PUBLIC cxx_std_23)

target_link_libraries(engine_lib
    PUBLIC
        logger_lib
)
