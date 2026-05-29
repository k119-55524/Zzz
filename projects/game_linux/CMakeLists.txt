add_executable(game_linux)

target_precompile_headers(game_linux PRIVATE
    pch/pch.h
)

target_include_directories(game_linux PRIVATE
    pch
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src
)

target_sources(game_linux
    PRIVATE
        main.h
        main.cpp
)

target_link_libraries(game_linux
    PRIVATE
        engine_lib
        logger_lib
)

target_compile_features(game_linux PUBLIC cxx_std_23)