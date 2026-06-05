add_executable(game_macos
    MACOSX_BUNDLE
    main.mm
)

target_include_directories(game_macos PRIVATE
    pch
    ${CMAKE_CURRENT_SOURCE_DIR}
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
    XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER com.zzz.game
    XCODE_SCHEME_ENVIRONMENT "OS_ACTIVITY_MODE=disable"
)

target_link_options(game_macos PRIVATE "-ObjC")
