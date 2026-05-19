add_library(game_android SHARED)

target_precompile_headers(game_android PRIVATE
    pch/pch.h
)

target_include_directories(game_android PRIVATE
    pch
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src
)

target_sources(game_android PRIVATE
    src/android_main.cpp
)

target_link_libraries(game_android PRIVATE
    engine_lib
    logger_lib

    android
    log
)

target_compile_features(game_android PUBLIC cxx_std_23)