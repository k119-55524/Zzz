cmake_minimum_required(VERSION 3.16)
project(zzz_user_scripts LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(Z_DEVELOPMENT_BUILD)
    add_compile_definitions(Z_DEVELOPMENT_BUILD=1)
endif()

file(GLOB_RECURSE SCRIPT_SOURCES "Assets/Scripts/*.cpp" "Assets/Scripts/*.h" "Assets/Scripts/*.hpp")

# Сборка статической библиотеки zzz_user_scripts (.lib)
add_library(zzz_user_scripts STATIC
    RegisterAllScripts.cpp
    ${SCRIPT_SOURCES}
)

set_target_properties(zzz_user_scripts PROPERTIES
    OUTPUT_NAME "zzz_user_scripts"
    PREFIX ""
)

get_filename_component(WORKSPACE_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../.." ABSOLUTE)

target_include_directories(zzz_user_scripts PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/Assets/Scripts
    "${WORKSPACE_SRC_DIR}/engine"
    "${WORKSPACE_SRC_DIR}/engine"
    "${WORKSPACE_SRC_DIR}/core/userscripts"
    "${WORKSPACE_SRC_DIR}/core/userscripts/base_script"
    "${WORKSPACE_SRC_DIR}"
)

if((WIN32 AND Z_FORCE_VULKAN) OR LINUX OR ANDROID)
    find_package(Vulkan REQUIRED)
    target_link_libraries(zzz_user_scripts PUBLIC Vulkan::Vulkan)
endif()

