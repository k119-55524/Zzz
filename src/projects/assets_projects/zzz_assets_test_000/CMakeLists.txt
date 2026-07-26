cmake_minimum_required(VERSION 3.16)
project(build_zzz_assets_test_000 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(Z_DEVELOPMENT_BUILD)
    add_compile_definitions(Z_DEVELOPMENT_BUILD=1)
endif()

file(GLOB_RECURSE SCRIPT_SOURCES "Assets/Scripts/*.cpp" "Assets/Scripts/*.h" "Assets/Scripts/*.hpp")

add_library(${PROJECT_NAME} SHARED
    RegisterAllScripts.cpp
    ${SCRIPT_SOURCES}
)

target_include_directories(${PROJECT_NAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/Assets/Scripts
)
