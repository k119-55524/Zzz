cmake_minimum_required(VERSION 3.28)

include(FetchContent)

# -----------------------------------------------------------------------------
# 1. SPIRV-Cross (KhronosGroup/SPIRV-Cross)
# Библиотека трансляции SPIR-V байткода в MSL (Metal Shading Language)
# -----------------------------------------------------------------------------
set(SPIRV_CROSS_EXCEPTIONS_TO_ASSERTIONS ON CACHE BOOL "" FORCE)
set(SPIRV_CROSS_SHARED OFF CACHE BOOL "" FORCE)
set(SPIRV_CROSS_STATIC ON CACHE BOOL "" FORCE)
set(SPIRV_CROSS_CLI OFF CACHE BOOL "" FORCE)
set(SPIRV_CROSS_ENABLE_TESTS OFF CACHE BOOL "" FORCE)
set(SPIRV_CROSS_ENABLE_GLSL ON CACHE BOOL "" FORCE)
set(SPIRV_CROSS_ENABLE_HLSL OFF CACHE BOOL "" FORCE)
set(SPIRV_CROSS_ENABLE_MSL ON CACHE BOOL "" FORCE)
set(SPIRV_CROSS_ENABLE_CPP OFF CACHE BOOL "" FORCE)
set(SPIRV_CROSS_ENABLE_REFLECT OFF CACHE BOOL "" FORCE)
set(SPIRV_CROSS_ENABLE_C_API OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    spirv_cross
    GIT_REPOSITORY https://github.com/KhronosGroup/SPIRV-Cross.git
    GIT_TAG        ${Z_SPIRV_CROSS_VERSION}
    UPDATE_DISCONNECTED TRUE
)

FetchContent_MakeAvailable(spirv_cross)

# -----------------------------------------------------------------------------
# 2. shader_builder target
# -----------------------------------------------------------------------------
add_library(shader_builder STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/pch/pch.h
    ${CMAKE_CURRENT_SOURCE_DIR}/pch/pch.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ShaderBuilderTypes.h
    ${CMAKE_CURRENT_SOURCE_DIR}/ShaderBuilder.h
    ${CMAKE_CURRENT_SOURCE_DIR}/ShaderBuilder.cpp
)

target_precompile_headers(shader_builder PRIVATE
    "pch/pch.h"
)

target_include_directories(shader_builder PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(shader_builder PUBLIC
    core_lib
)

# Подключение DXC (DirectX Shader Compiler)
if(TARGET Microsoft::DXC)
    target_link_libraries(shader_builder PRIVATE Microsoft::DXC)
elseif(TARGET DirectXShaderCompiler)
    target_link_libraries(shader_builder PRIVATE DirectXShaderCompiler)
endif()

# Подключение SPIRV-Cross (msl + core)
if(TARGET spirv-cross-msl)
    target_link_libraries(shader_builder PRIVATE spirv-cross-msl spirv-cross-core)
elseif(TARGET spirv-cross-static)
    target_link_libraries(shader_builder PRIVATE spirv-cross-static)
endif()

target_compile_features(shader_builder PUBLIC cxx_std_23)
