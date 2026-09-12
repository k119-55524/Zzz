cmake_minimum_required(VERSION 3.28)

project(Zzz LANGUAGES C CXX)

if(APPLE)
    enable_language(OBJCXX)
    add_link_options("-Wl,-no_warn_duplicate_libraries")
endif()

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/dist/$<CONFIG>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/dist/$<CONFIG>")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/dist/$<CONFIG>")

set(FETCHCONTENT_BASE_DIR "${CMAKE_SOURCE_DIR}/.cache" CACHE PATH "Directory for downloaded external packages")

# Если зависимости уже скачаны в .cache/ — включаем FULLY_DISCONNECTED (0 задержек, без subbuild).
# Если кэша ещё нет (первый запуск на чистой машине) — скачиваем один раз через UPDATES_DISCONNECTED.
if(EXISTS "${FETCHCONTENT_BASE_DIR}/googletest-src" AND EXISTS "${FETCHCONTENT_BASE_DIR}/googlebenchmark-src")
    set(FETCHCONTENT_FULLY_DISCONNECTED ON CACHE BOOL "Fully disconnect FetchContent when dependencies exist" FORCE)
else()
    set(FETCHCONTENT_FULLY_DISCONNECTED OFF CACHE BOOL "Allow download when dependencies missing" FORCE)
    set(FETCHCONTENT_UPDATES_DISCONNECTED ON CACHE BOOL "Skip update step for already-populated FetchContent dependencies" FORCE)
endif()


# Отключаем сканирование модулей C++20, так как они не используются в проекте и вызывают спам иерархии зависимостей в Ninja
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)

if(WIN32)
    add_compile_definitions(WIN32_LEAN_AND_MEAN NOMINMAX)
endif()

if(MSVC)
    add_compile_options(/utf-8 /Zc:preprocessor /W4 /w44062 /EHsc /wd4251)
endif()

include(package_versions.cmake)
include(build_configs/current.cmake OPTIONAL)

# Единый флаг Z_FORCE_VULKAN приходит из build_configs/current.cmake (BuildConfigurator/Switcher)
# как CMake-переменная; здесь же он пробрасывается в компилятор как define для Defines.h,
# чтобы не заводить для одного и того же выбора отдельный CMake- и compile-define флаг.
if(Z_FORCE_VULKAN)
    add_compile_definitions(Z_FORCE_VULKAN=1)
endif()

include(build_configs/setup_d3d12_packages.cmake OPTIONAL)

# Multi-config generators (Xcode) need to know about our custom configurations up front;
# single-config generators (Ninja) just ignore this and use CMAKE_BUILD_TYPE instead.
set(CMAKE_CONFIGURATION_TYPES "Debug;Development;Development_ASan;Development_TSan;Release" CACHE STRING "" FORCE)
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Choose the type of build." FORCE)
endif()

set(Z_DOTNET_CONFIG "$<IF:$<OR:$<CONFIG:Release>,$<CONFIG:MinSizeRel>,$<CONFIG:RelWithDebInfo>>,Release,Debug>")

string(TOUPPER "${CMAKE_BUILD_TYPE}" CMAKE_BUILD_TYPE_UPPER)
if(CMAKE_BUILD_TYPE_UPPER MATCHES "^DEVELOPMENT")
    add_compile_definitions(Z_DEVELOPMENT_BUILD=1)
endif()

# -----------------------------------------------------------------------------
# Per-configuration compiler/linker flags
# -----------------------------------------------------------------------------
if(MSVC)
    set(CMAKE_CXX_FLAGS_DEVELOPMENT "/O2 /Zi" CACHE STRING "" FORCE)
    set(CMAKE_C_FLAGS_DEVELOPMENT "/O2 /Zi" CACHE STRING "" FORCE)
    set(CMAKE_EXE_LINKER_FLAGS_DEVELOPMENT "/debug /INCREMENTAL:NO" CACHE STRING "" FORCE)
    set(CMAKE_SHARED_LINKER_FLAGS_DEVELOPMENT "/debug /INCREMENTAL:NO" CACHE STRING "" FORCE)

    set(CMAKE_CXX_FLAGS_DEVELOPMENT_ASAN "/O1 /Zi /fsanitize=address" CACHE STRING "" FORCE)
    set(CMAKE_C_FLAGS_DEVELOPMENT_ASAN "/O1 /Zi /fsanitize=address" CACHE STRING "" FORCE)
    set(CMAKE_EXE_LINKER_FLAGS_DEVELOPMENT_ASAN "/debug /INCREMENTAL:NO" CACHE STRING "" FORCE)
    set(CMAKE_SHARED_LINKER_FLAGS_DEVELOPMENT_ASAN "/debug /INCREMENTAL:NO" CACHE STRING "" FORCE)

    set(CMAKE_CXX_FLAGS_RELEASE "/O2 /DNDEBUG" CACHE STRING "" FORCE)
    set(CMAKE_C_FLAGS_RELEASE "/O2 /DNDEBUG" CACHE STRING "" FORCE)

    # ThreadSanitizer is not supported by MSVC. Development_TSan has no flags here
    # and its preset is hidden on Windows (see CMakePresets.json).
else()
    set(CMAKE_CXX_FLAGS_DEVELOPMENT "-O2 -g" CACHE STRING "" FORCE)
    set(CMAKE_C_FLAGS_DEVELOPMENT "-O2 -g" CACHE STRING "" FORCE)

    set(CMAKE_CXX_FLAGS_DEVELOPMENT_ASAN "-O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer" CACHE STRING "" FORCE)
    set(CMAKE_C_FLAGS_DEVELOPMENT_ASAN "-O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer" CACHE STRING "" FORCE)
    set(CMAKE_EXE_LINKER_FLAGS_DEVELOPMENT_ASAN "-fsanitize=address,undefined" CACHE STRING "" FORCE)
    set(CMAKE_SHARED_LINKER_FLAGS_DEVELOPMENT_ASAN "-fsanitize=address,undefined" CACHE STRING "" FORCE)

    set(CMAKE_CXX_FLAGS_DEVELOPMENT_TSAN "-O1 -g -fsanitize=thread" CACHE STRING "" FORCE)
    set(CMAKE_C_FLAGS_DEVELOPMENT_TSAN "-O1 -g -fsanitize=thread" CACHE STRING "" FORCE)
    set(CMAKE_EXE_LINKER_FLAGS_DEVELOPMENT_TSAN "-fsanitize=thread" CACHE STRING "" FORCE)
    set(CMAKE_SHARED_LINKER_FLAGS_DEVELOPMENT_TSAN "-fsanitize=thread" CACHE STRING "" FORCE)

    # Project default is -O2 for Release (Clang/GCC otherwise default to -O3).
    set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG" CACHE STRING "" FORCE)
    set(CMAKE_C_FLAGS_RELEASE "-O2 -DNDEBUG" CACHE STRING "" FORCE)
endif()

add_subdirectory(src/math)
add_subdirectory(src/logger)
add_subdirectory(src/core)

add_subdirectory(src/tools/RemoteLogViewer)
add_subdirectory(src/engine)
add_subdirectory(src/server)
add_subdirectory(src/projects/assets_projects/zzz_assets_test_000)



if(ANDROID)
    add_subdirectory(src/projects/game_android)
elseif(WIN32)
    add_subdirectory(src/editor_dll)
    add_subdirectory(src/tools/assets_builder/assets_builder_dll)
    add_subdirectory(src/projects/game_win)

    # Авто-сборка C# AssetsBuilderGUI в CMake с зависимостью от C++ assets_builder_dll
    add_custom_target(assets_builder_gui_build ALL
        COMMAND dotnet build "${CMAKE_SOURCE_DIR}/src/tools/assets_builder/assets_builder_gui/assets_builder_gui.csproj" -c ${Z_DOTNET_CONFIG}
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
        COMMENT "Building Assets Builder C# GUI..."
        VERBATIM
    )
    add_dependencies(assets_builder_gui_build assets_builder_dll)
elseif(IOS)
    add_subdirectory(src/projects/game_ios)
elseif(APPLE)
    add_subdirectory(src/projects/game_macos)
elseif(UNIX)
    add_subdirectory(src/projects/game_linux)
endif()

# -----------------------------------------------------------------------------
# Tests & Benchmarks
# Z_ADD_PROJECT_TESTS_IN_BUILD / Z_ADD_PROJECT_BENCHMARKS_IN_BUILD are set by
# build_configs/current.cmake (generated by the external BuildConfigurator tool).
# -----------------------------------------------------------------------------
if(Z_ADD_PROJECT_TESTS_IN_BUILD)
    enable_testing()
    add_subdirectory(src/qa/tests)
endif()

if(Z_ADD_PROJECT_BENCHMARKS_IN_BUILD)
    add_subdirectory(src/qa/benchmark)
endif()

# -----------------------------------------------------------------------------
# Documentation (Doxygen)
# -----------------------------------------------------------------------------
include(automation/doxygen/docs.cmake)
