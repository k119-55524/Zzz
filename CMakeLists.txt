cmake_minimum_required(VERSION 3.28)

project(Zzz LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(ZZZ_ENABLE_LOGGER "Enable logger" ON)

add_subdirectory(src/logger/logger_lib)