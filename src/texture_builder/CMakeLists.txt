cmake_minimum_required(VERSION 3.28)

include(FetchContent)

# -----------------------------------------------------------------------------
# 1. stb (nothings/stb) - Header-only library
# -----------------------------------------------------------------------------
FetchContent_Declare(
    stb
    GIT_REPOSITORY https://github.com/nothings/stb.git
    GIT_TAG        ${Z_STB_VERSION}
    UPDATE_DISCONNECTED TRUE
)

FetchContent_MakeAvailable(stb)

# -----------------------------------------------------------------------------
# 2. DirectXTex (microsoft/DirectXTex) - Desktop only (Windows, Linux, macOS)
# -----------------------------------------------------------------------------
if(NOT ANDROID AND NOT IOS)
    set(BUILD_TOOLS OFF CACHE BOOL "Build DirectXTex tools" FORCE)
    set(BUILD_SAMPLE OFF CACHE BOOL "Build DirectXTex samples" FORCE)
    set(BUILD_DX11 OFF CACHE BOOL "Build DirectXTex DX11 components and shaders" FORCE)
    set(BUILD_DX12 OFF CACHE BOOL "Build DirectXTex DX12 components" FORCE)

    FetchContent_Declare(
        directxtex
        GIT_REPOSITORY https://github.com/microsoft/DirectXTex.git
        GIT_TAG        ${Z_DIRECTXTEX_VERSION}
        UPDATE_DISCONNECTED TRUE
    )

    FetchContent_MakeAvailable(directxtex)
endif()

# -----------------------------------------------------------------------------
# 3. texture_builder library target
# -----------------------------------------------------------------------------
add_library(texture_builder STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/pch/pch.h
    ${CMAKE_CURRENT_SOURCE_DIR}/pch/pch.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/stb_impl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/TextureBuilderTypes.h
    ${CMAKE_CURRENT_SOURCE_DIR}/TextureBuilder.h
    ${CMAKE_CURRENT_SOURCE_DIR}/TextureBuilder.cpp
)

target_precompile_headers(texture_builder PRIVATE
    "pch/pch.h"
)

target_include_directories(texture_builder PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${stb_SOURCE_DIR}
)

target_link_libraries(texture_builder PUBLIC
    core_lib
)

if(TARGET DirectXTex)
    target_link_libraries(texture_builder PRIVATE
        DirectXTex
    )
endif()

target_compile_features(texture_builder PUBLIC cxx_std_23)
