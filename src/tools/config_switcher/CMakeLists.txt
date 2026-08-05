cmake_minimum_required(VERSION 3.28)
project(build_configurator_switch LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(build_configurator_switch main.cpp)
target_include_directories(build_configurator_switch PRIVATE "${CMAKE_SOURCE_DIR}/src/tools/assets_builder/assets_builder_dll")

add_custom_command(TARGET build_configurator_switch POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        "$<TARGET_FILE:build_configurator_switch>"
        "${CMAKE_CURRENT_SOURCE_DIR}/../../../build_configs/$<TARGET_FILE_NAME:build_configurator_switch>"
    COMMENT "Copying build_configurator_switch to build_configs"
)
