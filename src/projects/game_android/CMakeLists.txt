find_package(game-activity REQUIRED CONFIG)

add_library(game_android SHARED)

target_precompile_headers(game_android PRIVATE
    "$<$<COMPILE_LANGUAGE:CXX>:pch/pch.h>"
)

target_include_directories(game_android PRIVATE
    pch
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_sources(game_android PRIVATE
    pch/pch.h
    pch/pch.cpp
    src/android_main.h
    src/android_main.cpp
)

target_link_libraries(game_android PRIVATE
    engine_lib
    logger_lib

    # Это магическая строка, которая решит UnsatisfiedLinkError
    -Wl,--whole-archive game-activity::game-activity_static -Wl,--no-whole-archive

    android
    log
)

target_compile_features(game_android PUBLIC cxx_std_23)
