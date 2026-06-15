add_library(common_lib STATIC)

target_include_directories(common_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_sources(common_lib PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/common.h
    ${CMAKE_CURRENT_SOURCE_DIR}/constants.h
    ${CMAKE_CURRENT_SOURCE_DIR}/platform_defines.h
    ${CMAKE_CURRENT_SOURCE_DIR}/ensure.h
    ${CMAKE_CURRENT_SOURCE_DIR}/macroses.h
    ${CMAKE_CURRENT_SOURCE_DIR}/memory_utils.h
    ${CMAKE_CURRENT_SOURCE_DIR}/throw_wrappers.h
    ${CMAKE_CURRENT_SOURCE_DIR}/throw_wrappers.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/types.h
    ${CMAKE_CURRENT_SOURCE_DIR}/enums/eLogMessageType.h
    ${CMAKE_CURRENT_SOURCE_DIR}/enums/eWinResize.h
    ${CMAKE_CURRENT_SOURCE_DIR}/enums/enum_to_string.h
    ${CMAKE_CURRENT_SOURCE_DIR}/serialize/Serializer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/serialize/Serializer.cpp
)

target_compile_features(common_lib PUBLIC cxx_std_23)

target_link_libraries(common_lib PUBLIC logger_lib)
