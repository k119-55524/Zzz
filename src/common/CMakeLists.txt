macro(setup_common_target TARGET_NAME IS_EDITOR)
    add_library(${TARGET_NAME} STATIC)

    target_include_directories(${TARGET_NAME} PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/..
    )

    target_sources(${TARGET_NAME} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Common.h
        ${CMAKE_CURRENT_SOURCE_DIR}/Defines.h
        ${CMAKE_CURRENT_SOURCE_DIR}/Ensure.h
        ${CMAKE_CURRENT_SOURCE_DIR}/CommonExport.h
        ${CMAKE_CURRENT_SOURCE_DIR}/Macroses.h
        ${CMAKE_CURRENT_SOURCE_DIR}/MemoryUtils.h
        ${CMAKE_CURRENT_SOURCE_DIR}/ThrowWrappers.h
        ${CMAKE_CURRENT_SOURCE_DIR}/ThrowWrappers.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Enums/eLogMessageType.h
        ${CMAKE_CURRENT_SOURCE_DIR}/Enums/eWinResize.h
        ${CMAKE_CURRENT_SOURCE_DIR}/Templates/DoubleBufferedVector.h
        ${CMAKE_CURRENT_SOURCE_DIR}/Templates/Size2D.h
    )

    target_compile_features(${TARGET_NAME} PUBLIC cxx_std_23)
    target_compile_definitions(${TARGET_NAME} PRIVATE Z_TARGET_NAME="${TARGET_NAME}")

    if(${IS_EDITOR})
        # EDITORDLL_EXPORTS должен совпадать с тем же define в editor_dll.vcxproj/CMakeLists.txt -
        # common_lib_editor статический, но его символы (Z_COMMON_API, например
        # throw_runtime_error) линкуются только в editor_dll.dll, иначе Z_COMMON_API соберёт здесь
        # dllimport вместо dllexport (см. CommonExport.h).
        target_compile_definitions(${TARGET_NAME} PUBLIC Z_EDITOR=1 EDITORDLL_EXPORTS)
    endif()

    export_to_editor_sdk(${TARGET_NAME} "${CMAKE_CURRENT_SOURCE_DIR}" "common")
endmacro()

setup_common_target(common_lib FALSE)
setup_common_target(common_lib_editor TRUE)
