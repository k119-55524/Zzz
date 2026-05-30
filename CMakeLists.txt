cmake_minimum_required(VERSION 3.28)

project(Zzz LANGUAGES C CXX)

if(APPLE)
    enable_language(OBJCXX)
endif()

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    add_compile_options(/Zc:preprocessor)
endif()

include(build_configs/current.cmake OPTIONAL)

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
