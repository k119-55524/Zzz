add_library(common_lib INTERFACE)

target_include_directories(common_lib INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_sources(common_lib INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/common.h
    ${CMAKE_CURRENT_SOURCE_DIR}/platform_defines.h
    ${CMAKE_CURRENT_SOURCE_DIR}/ensure.h
    ${CMAKE_CURRENT_SOURCE_DIR}/core_macros.h
    ${CMAKE_CURRENT_SOURCE_DIR}/memory_utils.h
    ${CMAKE_CURRENT_SOURCE_DIR}/types.h
)

target_compile_features(common_lib INTERFACE cxx_std_23)
