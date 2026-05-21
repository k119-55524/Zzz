add_executable(game_macos
    MACOSX_BUNDLE
)

target_precompile_headers(game_macos PRIVATE
    pch/pch.h
)

target_include_directories(game_macos PRIVATE
    pch
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src
)

target_sources(game_macos
    PRIVATE
        main.mm
        pch/pch.mm
)

target_link_libraries(game_macos
    PRIVATE
        engine_lib
        logger_lib
)

target_compile_features(game_macos PUBLIC cxx_std_23)

find_library(COCOA_LIBRARY Cocoa)
find_library(METAL_LIBRARY Metal)
find_library(METALKIT_LIBRARY MetalKit)
find_library(QUARTZCORE_LIBRARY QuartzCore)

target_link_libraries(game_macos
    PRIVATE
        ${COCOA_LIBRARY}
        ${METAL_LIBRARY}
        ${METALKIT_LIBRARY}
        ${QUARTZCORE_LIBRARY}
)

set_target_properties(game_macos PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER com.zzz.game
    MACOSX_BUNDLE_BUNDLE_NAME Zzz
)