cmake_minimum_required(VERSION 3.28)
project(build_configurator_switch LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(JSON_HPP "${CMAKE_CURRENT_BINARY_DIR}/json.hpp")
if(NOT EXISTS "${JSON_HPP}")
    message(STATUS "Downloading nlohmann/json 3.11.3...")
    file(DOWNLOAD
        "https://github.com/nlohmann/json/releases/download/v3.11.3/json.hpp"
        "${JSON_HPP}"
        SHOW_PROGRESS
        STATUS download_status
    )
    list(GET download_status 0 status_code)
    if(NOT status_code EQUAL 0)
        message(FATAL_ERROR
            "Не удалось скачать json.hpp. "
            "Проверьте интернет-соединение или разместите json.hpp вручную в: ${CMAKE_CURRENT_BINARY_DIR}/")
    endif()
endif()

add_executable(build_configurator_switch main.cpp)
target_include_directories(build_configurator_switch PRIVATE "${CMAKE_CURRENT_BINARY_DIR}")

add_custom_command(TARGET build_configurator_switch POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        "$<TARGET_FILE:build_configurator_switch>"
        "${CMAKE_CURRENT_SOURCE_DIR}/../../../build_configs/$<TARGET_FILE_NAME:build_configurator_switch>"
    COMMENT "Copying build_configurator_switch to build_configs"
)
