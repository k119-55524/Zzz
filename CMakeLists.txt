cmake_minimum_required(VERSION 3.28)

project(Zzz LANGUAGES C CXX)

if(APPLE)
    enable_language(OBJCXX)
    add_link_options("-Wl,-no_warn_duplicate_libraries")
endif()

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    add_compile_options(/Zc:preprocessor /W4 /w44062)
else()
    add_compile_options(-Wswitch-enum)
endif()

include(build_configs/current.cmake OPTIONAL)

string(TOUPPER "${CMAKE_BUILD_TYPE}" CMAKE_BUILD_TYPE_UPPER)
if(CMAKE_BUILD_TYPE_UPPER STREQUAL "DEVELOPMENT")
    add_compile_definitions(Z_DEVELOPMENT_BUILD=1)
endif()

add_subdirectory(src/logger/logger_lib)
add_subdirectory(src/engine)

if(ANDROID)
    add_subdirectory(projects/game_android)
elseif(WIN32)
    add_subdirectory(projects/game_win)
elseif(IOS)
    add_subdirectory(projects/game_ios)
elseif(APPLE)
    add_subdirectory(projects/game_macos)
elseif(UNIX)
    add_subdirectory(projects/game_linux)
endif()

# -----------------------------------------------------------------------------
# Options
# -----------------------------------------------------------------------------
option(Z_BUILD_TESTS "Build tests" OFF)
option(Z_BUILD_BENCHMARKS "Build benchmarks" OFF)
option(Z_USE_ASAN "Enable AddressSanitizer (ASan)" OFF)

if(Z_USE_ASAN)
    if(MSVC)
        add_compile_options(/fsanitize=address)
        add_link_options(/INFERASANLIBS)
    else()
        add_compile_options(-fsanitize=address)
        add_link_options(-fsanitize=address)
    endif()
endif()

# -----------------------------------------------------------------------------
# Tests & Benchmarks
# -----------------------------------------------------------------------------
if(Z_BUILD_TESTS OR Z_BUILD_BENCHMARKS)
    enable_testing()
endif()

if(Z_BUILD_TESTS)
    add_subdirectory(tests)
endif()

if(Z_BUILD_BENCHMARKS)
    add_subdirectory(benchmarks)
endif()
