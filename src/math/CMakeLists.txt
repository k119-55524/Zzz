macro(setup_math_target TARGET_NAME IS_EDITOR)
    add_library(${TARGET_NAME} INTERFACE)

    target_include_directories(${TARGET_NAME} INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/..
    )

    target_compile_features(${TARGET_NAME} INTERFACE cxx_std_23)
endmacro()

setup_math_target(math FALSE)
setup_math_target(math_editor TRUE)
