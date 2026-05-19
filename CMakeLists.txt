cmake_minimum_required(VERSION 3.28)

project(Zzz LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(ZZZ_ENABLE_LOGGER "Enable logger" ON)

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

#if(APPLE)
#    add_subdirectory(projects/game_macos)
#endif()