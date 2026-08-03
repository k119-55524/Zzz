set(JSON_HPP "${CMAKE_CURRENT_BINARY_DIR}/json.hpp")
if(NOT EXISTS "${JSON_HPP}")
    message(STATUS "Downloading nlohmann/json 3.11.3 for assets_builder_dll...")
    file(DOWNLOAD
        "https://github.com/nlohmann/json/releases/download/v3.11.3/json.hpp"
        "${JSON_HPP}"
        STATUS download_status
    )
endif()

add_library(assets_builder_dll SHARED
    BuilderExport.h
    BuilderApi.h
    BuilderApi.cpp
    PackagePacker.h
    PackagePacker.cpp
)

target_include_directories(assets_builder_dll PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    "${CMAKE_SOURCE_DIR}/src"
    "${CMAKE_SOURCE_DIR}/src/common"
    "${CMAKE_SOURCE_DIR}/src/engine"
)

target_link_libraries(assets_builder_dll PRIVATE
    common_lib
    core_lib
    logger_lib
)

target_compile_definitions(assets_builder_dll PRIVATE ASSETS_BUILDER_EXPORTS Z_TARGET_NAME="assets_builder_dll")
target_compile_features(assets_builder_dll PUBLIC cxx_std_23)
