cmake_minimum_required(VERSION 3.28)

project(Zzz LANGUAGES C CXX)

if(APPLE)
    enable_language(OBJCXX)
endif()

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(build_configs/current.cmake OPTIONAL)

add_subdirectory(src/logger/logger_lib)
add_subdirectory(src/engine)

if(WIN32)
    add_subdirectory(projects/game_win)
endif()

if(UNIX AND NOT APPLE)
    add_subdirectory(projects/game_linux)
endif()

if(ANDROID)
    add_subdirectory(projects/game_android)
endif()

if(APPLE AND NOT IOS)
    add_subdirectory(projects/game_macos)
endif()

if(IOS)
    add_subdirectory(projects/game_ios)
endif()