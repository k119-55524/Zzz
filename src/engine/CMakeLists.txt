add_library(engine_lib)

target_precompile_headers(engine_lib PRIVATE
    pch/pch.h
)

target_include_directories(engine_lib PRIVATE
    pch
    ${CMAKE_SOURCE_DIR}/src
)

target_sources(engine_lib
    PUBLIC
        FILE_SET CXX_MODULES FILES
            modules/engine.ixx
)

target_sources(engine_lib
    PRIVATE
        src/engine.cpp
)

target_compile_features(engine_lib PUBLIC cxx_std_23)

target_link_libraries(engine_lib
    PUBLIC
        logger_lib
)