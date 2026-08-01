macro(setup_core_target TARGET_NAME IS_EDITOR)
    add_library(${TARGET_NAME} STATIC)

    target_include_directories(${TARGET_NAME} PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/..
    )

    target_sources(${TARGET_NAME} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/constants.h
        ${CMAKE_CURRENT_SOURCE_DIR}/Guid.h
        ${CMAKE_CURRENT_SOURCE_DIR}/version.h
        ${CMAKE_CURRENT_SOURCE_DIR}/enums/ePackage.h
        ${CMAKE_CURRENT_SOURCE_DIR}/enums/eEnumToString.h
        ${CMAKE_CURRENT_SOURCE_DIR}/io/FileHeader.h
        ${CMAKE_CURRENT_SOURCE_DIR}/io/gamepackage/PackageHeader.h
        ${CMAKE_CURRENT_SOURCE_DIR}/io/gamepackage/PackageEntry.h
        ${CMAKE_CURRENT_SOURCE_DIR}/io/gamepackage/ViewData.h
        ${CMAKE_CURRENT_SOURCE_DIR}/io/gamepackage/ProjectManifestData.h
        ${CMAKE_CURRENT_SOURCE_DIR}/io/gamepackage/SceneData.h
        ${CMAKE_CURRENT_SOURCE_DIR}/io/gamepackage/PrefabData.h
        ${CMAKE_CURRENT_SOURCE_DIR}/serialize/Serializer.h
        ${CMAKE_CURRENT_SOURCE_DIR}/serialize/Serializer.cpp
    )

    target_compile_features(${TARGET_NAME} PUBLIC cxx_std_23)
    target_compile_definitions(${TARGET_NAME} PRIVATE Z_TARGET_NAME="${TARGET_NAME}")

    if(${IS_EDITOR})
        target_compile_definitions(${TARGET_NAME} PUBLIC Z_EDITOR=1 EDITORDLL_EXPORTS)
    endif()

    export_to_editor_sdk(${TARGET_NAME} "${CMAKE_CURRENT_SOURCE_DIR}" "core")
endmacro()

setup_core_target(core_lib FALSE)
setup_core_target(core_lib_editor TRUE)
