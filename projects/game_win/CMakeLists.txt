add_executable(game_win WIN32)

target_precompile_headers(game_win PRIVATE
    pch/pch.h
)

target_include_directories(game_win PRIVATE
    pch
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_sources(game_win PRIVATE
    pch/pch.cpp
    main.h
    main.cpp
    resource/app.rc
)

target_link_libraries(game_win PRIVATE
    engine_lib
    logger_lib
)

target_compile_features(game_win PUBLIC cxx_std_23)