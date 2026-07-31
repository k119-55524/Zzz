macro(setup_common_target TARGET_NAME IS_EDITOR)
    add_library(${TARGET_NAME} STATIC)

    target_include_directories(${TARGET_NAME} PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/..
    )

    target_sources(${TARGET_NAME} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/package/PackageHeader.h
        ${CMAKE_CURRENT_SOURCE_DIR}/package/PackageEntry.h
        ${CMAKE_CURRENT_SOURCE_DIR}/package/ViewData.h
        ${CMAKE_CURRENT_SOURCE_DIR}/package/ProjectManifestData.h
        ${CMAKE_CURRENT_SOURCE_DIR}/package/SceneData.h
        ${CMAKE_CURRENT_SOURCE_DIR}/common.h
        ${CMAKE_CURRENT_SOURCE_DIR}/constants.h
        ${CMAKE_CURRENT_SOURCE_DIR}/defines.h
        ${CMAKE_CURRENT_SOURCE_DIR}/ensure.h
        ${CMAKE_CURRENT_SOURCE_DIR}/CommonExport.h
        ${CMAKE_CURRENT_SOURCE_DIR}/macroses.h
        ${CMAKE_CURRENT_SOURCE_DIR}/memory_utils.h
        ${CMAKE_CURRENT_SOURCE_DIR}/throw_wrappers.h
        ${CMAKE_CURRENT_SOURCE_DIR}/throw_wrappers.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/types.h
        ${CMAKE_CURRENT_SOURCE_DIR}/enums/eLogMessageType.h
        ${CMAKE_CURRENT_SOURCE_DIR}/enums/eWinResize.h
        ${CMAKE_CURRENT_SOURCE_DIR}/enums/ePackage.h
        ${CMAKE_CURRENT_SOURCE_DIR}/enums/eEnumToString.h
        ${CMAKE_CURRENT_SOURCE_DIR}/io/zFileHeader.h
        ${CMAKE_CURRENT_SOURCE_DIR}/guid.h
        ${CMAKE_CURRENT_SOURCE_DIR}/serialize/Serializer.h
        ${CMAKE_CURRENT_SOURCE_DIR}/serialize/Serializer.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/templates/DoubleBufferedVector.h
        ${CMAKE_CURRENT_SOURCE_DIR}/templates/Size2D.h
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
