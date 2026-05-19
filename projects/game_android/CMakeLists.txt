add_library(game_android SHARED)

# Находим native_app_glue в NDK
set(NATIVE_APP_GLUE_DIR "${ANDROID_NDK}/sources/android/native_app_glue")
add_library(native_app_glue STATIC
    ${NATIVE_APP_GLUE_DIR}/android_native_app_glue.c
)
target_include_directories(native_app_glue PUBLIC ${NATIVE_APP_GLUE_DIR})

target_precompile_headers(game_android PRIVATE
    pch/pch.h
)

target_include_directories(game_android PRIVATE
    pch
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src
)

target_sources(game_android PRIVATE
    src/android_main.cpp
)

target_link_libraries(game_android PRIVATE
    native_app_glue
    engine_lib
    logger_lib

    android
    log
)

target_compile_features(game_android PUBLIC cxx_std_23)