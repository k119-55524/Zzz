cmake_minimum_required(VERSION 3.28)

project(Zzz LANGUAGES C CXX)

if(APPLE)
    enable_language(OBJCXX)
    add_link_options("-Wl,-no_warn_duplicate_libraries")
endif()

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/$<CONFIG>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/$<CONFIG>")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/$<CONFIG>")

if(MSVC)
    add_compile_options(/Zc:preprocessor /W4 /w44062 /EHsc /wd4251)
else()
    add_compile_options(-Wswitch-enum)
endif()

include(package_versions.cmake)
include(build_configs/current.cmake OPTIONAL)
include(build_configs/setup_d3d12_packages.cmake OPTIONAL)

# Multi-config generators (Xcode) need to know about our custom configurations up front;
# single-config generators (Ninja) just ignore this and use CMAKE_BUILD_TYPE instead.
set(CMAKE_CONFIGURATION_TYPES "Debug;Development;Development_ASan;Development_TSan;Release" CACHE STRING "" FORCE)

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

# -----------------------------------------------------------------------------
# Автоматический экспорт SDK в папку редактора
# -----------------------------------------------------------------------------
macro(export_to_editor_sdk TARGET_NAME SRC_INCLUDE_DIR DST_SUBDIR)
    set(SDK_DIR "${CMAKE_SOURCE_DIR}/bin/$<CONFIG>/libs/zlibs")
    
    # Копируем .dll (если это shared библиотека на Windows) или .so (на Linux)
    add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "${SDK_DIR}/bin"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:${TARGET_NAME}> "${SDK_DIR}/bin/"
    )
    
    # Копируем .lib / .a
    add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "${SDK_DIR}/lib"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_LINKER_FILE:${TARGET_NAME}> "${SDK_DIR}/lib/"
    )

    # Копируем только .h файлы через внешний скрипт
    add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -DSRC_DIR="${SRC_INCLUDE_DIR}" -DDST_DIR="${SDK_DIR}/include/${DST_SUBDIR}" -P "${CMAKE_SOURCE_DIR}/copy_headers.cmake"
    )
endmacro()

add_subdirectory(src/core)
add_subdirectory(src/logger)

target_link_libraries(core_lib PUBLIC logger_lib)
target_link_libraries(core_lib_editor PUBLIC logger_lib_editor)

if(TARGET Microsoft::WinPixEventRuntime)
    target_link_libraries(core_lib PUBLIC Microsoft::WinPixEventRuntime)
    target_link_libraries(core_lib_editor PUBLIC Microsoft::WinPixEventRuntime)
endif()

if(TARGET Microsoft::DXC)
    target_link_libraries(core_lib PUBLIC Microsoft::DXC)
    target_link_libraries(core_lib_editor PUBLIC Microsoft::DXC)
endif()

add_subdirectory(src/tools/RemoteLogViewer)
add_subdirectory(src/engine)
add_subdirectory(src/projects/assets_projects/zzz_assets_test_000)


if(ANDROID)
    add_subdirectory(src/projects/game_android)
elseif(WIN32)
    add_subdirectory(src/editor_dll)
    add_subdirectory(src/tools/assets_builder/assets_builder_dll)
    add_subdirectory(src/projects/game_win)

    # Авто-сборка C# AssetsBuilderGUI в CMake с зависимостью от C++ assets_builder_dll
    add_custom_target(assets_builder_gui_build ALL
        COMMAND dotnet build "${CMAKE_SOURCE_DIR}/src/tools/assets_builder/assets_builder_gui/assets_builder_gui.csproj" -c $<CONFIG>
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
        COMMENT "Building Assets Builder C# GUI..."
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
