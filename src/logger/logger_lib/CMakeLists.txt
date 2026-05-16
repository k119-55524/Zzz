add_library(logger_lib)

target_precompile_headers(logger_lib PRIVATE
    src/pch.h
)

target_sources(logger_lib
    PUBLIC
    FILE_SET CXX_MODULES FILES
        include/logger.ixx

    PRIVATE
        src/logger.cpp
)

target_compile_features(logger_lib PUBLIC cxx_std_23)