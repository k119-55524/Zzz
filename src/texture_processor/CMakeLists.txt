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
# 2. DirectXTex (microsoft/DirectXTex)
# -----------------------------------------------------------------------------
set(BUILD_TOOLS OFF CACHE BOOL "Build DirectXTex tools" FORCE)
set(BUILD_SAMPLE OFF CACHE BOOL "Build DirectXTex samples" FORCE)

FetchContent_Declare(
    directxtex
    GIT_REPOSITORY https://github.com/microsoft/DirectXTex.git
    GIT_TAG        ${Z_DIRECTXTEX_VERSION}
    UPDATE_DISCONNECTED TRUE
)

FetchContent_MakeAvailable(directxtex)

# -----------------------------------------------------------------------------
# 3. texture_processor library target
# -----------------------------------------------------------------------------
add_library(texture_processor STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/TextureFormat.h
    ${CMAKE_CURRENT_SOURCE_DIR}/TextureProcessorTypes.h
    ${CMAKE_CURRENT_SOURCE_DIR}/TextureProcessor.h
    ${CMAKE_CURRENT_SOURCE_DIR}/TextureProcessor.cpp
)

target_include_directories(texture_processor PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${stb_SOURCE_DIR}
)

target_link_libraries(texture_processor PUBLIC
    DirectXTex
    core_lib
)

target_compile_features(texture_processor PUBLIC cxx_std_23)
