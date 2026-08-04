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
    "${WORKSPACE_SRC_DIR}/common"
    "${WORKSPACE_SRC_DIR}"
)

# Отображение файлов собранного пакета zzz_assets_test_000_build в дереве Visual Studio без их компиляции
get_filename_component(BUILD_OUTPUT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../zzz_assets_test_000_build" ABSOLUTE)

if(EXISTS "${BUILD_OUTPUT_DIR}")
    file(GLOB_RECURSE BUILD_OUTPUT_FILES CONFIGURE_DEPENDS "${BUILD_OUTPUT_DIR}/*")
    if(BUILD_OUTPUT_FILES)
        target_sources(zzz_user_scripts PRIVATE ${BUILD_OUTPUT_FILES})
        set_source_files_properties(${BUILD_OUTPUT_FILES} PROPERTIES HEADER_FILE_ONLY TRUE EXCLUDED_FROM_BUILD TRUE)
        source_group(TREE "${BUILD_OUTPUT_DIR}" PREFIX "Build Output" FILES ${BUILD_OUTPUT_FILES})
    endif()
endif()
