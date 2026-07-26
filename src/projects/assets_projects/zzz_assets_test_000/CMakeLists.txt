cmake_minimum_required(VERSION 3.20)

project(zzz_assets_test_000 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SOURCES
    RegisterAllScripts.cpp
    Assets/Scripts/SampleGameScript.cpp
    Assets/Scripts/SampleSceneScript.cpp
    Assets/Scripts/SampleViewScript.cpp
    Assets/Scripts/SampleObjectScript.cpp
)

set(HEADERS
    Assets/Scripts/SampleGameScript.h
    Assets/Scripts/SampleSceneScript.h
    Assets/Scripts/SampleViewScript.h
    Assets/Scripts/SampleObjectScript.h
)

add_library(${PROJECT_NAME} SHARED ${SOURCES} ${HEADERS})

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/Assets/Scripts
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(${PROJECT_NAME} PRIVATE engine_lib)

set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/$<CONFIG>"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/$<CONFIG>"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/$<CONFIG>"
)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_CURRENT_SOURCE_DIR}/Assets"
    "${CMAKE_BINARY_DIR}/bin/$<CONFIG>/Assets"
)
