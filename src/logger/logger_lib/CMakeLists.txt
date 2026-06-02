add_library(logger_lib)

target_precompile_headers(logger_lib PRIVATE
    pch/pch.h
)

target_include_directories(logger_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../../foundation
)

target_sources(logger_lib PRIVATE
    logger.h
    logger.cpp
    header.h
    "headers/enums.h")

target_compile_features(logger_lib PUBLIC cxx_std_23)

if(ZZZ_ENABLE_LOGGER)
    target_compile_definitions(logger_lib PUBLIC ZZZ_ENABLE_LOGGER=1)
endif()

if(WIN32)
    target_link_libraries(logger_lib PRIVATE User32)
endif()

if(ANDROID)
    target_link_libraries(logger_lib PRIVATE log)
endif()
