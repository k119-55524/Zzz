add_executable(game_ios
    main.mm
    AppDelegate.mm
    ViewController.mm
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