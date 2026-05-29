add_executable(game_ios
    MACOSX_BUNDLE
    main.mm
    AppDelegate.mm
    ViewController.mm
)

target_include_directories(game_ios PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(game_ios
    PRIVATE
        engine_lib
        logger_lib
)

find_library(UIKIT_LIBRARY UIKit)
find_library(METAL_LIBRARY Metal)
find_library(METALKIT_LIBRARY MetalKit)
find_library(FOUNDATION_LIBRARY Foundation)

target_link_libraries(game_ios
    PRIVATE
        ${UIKIT_LIBRARY}
        ${METAL_LIBRARY}
        ${METALKIT_LIBRARY}
        ${FOUNDATION_LIBRARY}
)

set_target_properties(game_ios PROPERTIES
    MACOSX_BUNDLE YES
    MACOSX_BUNDLE_GUI_IDENTIFIER com.dzzzz.game
    MACOSX_BUNDLE_BUNDLE_NAME Zzz

    XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER com.dzzzz.game
    XCODE_ATTRIBUTE_IPHONEOS_DEPLOYMENT_TARGET 17.0

    XCODE_ATTRIBUTE_CURRENT_PROJECT_VERSION 1
    XCODE_ATTRIBUTE_MARKETING_VERSION 1.0
)
