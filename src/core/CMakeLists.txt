macro(setup_core_target TARGET_NAME IS_EDITOR)
    add_library(${TARGET_NAME} STATIC)

    target_include_directories(${TARGET_NAME} PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/..
    )

    target_sources(${TARGET_NAME} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Types.h
        ${CMAKE_CURRENT_SOURCE_DIR}/Fwd.h
        ${CMAKE_CURRENT_SOURCE_DIR}/Constants.h
        ${CMAKE_CURRENT_SOURCE_DIR}/Guid.h
        ${CMAKE_CURRENT_SOURCE_DIR}/Version.h
        ${CMAKE_CURRENT_SOURCE_DIR}/Enums/ePackage.h
        ${CMAKE_CURRENT_SOURCE_DIR}/Enums/eEnumToString.h
        ${CMAKE_CURRENT_SOURCE_DIR}/IO/FileHeader.h
        ${CMAKE_CURRENT_SOURCE_DIR}/IO/Config/ConfigManager.h
        ${CMAKE_CURRENT_SOURCE_DIR}/IO/Config/ConfigManager.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/IO/Config/EngineConfig.h
        ${CMAKE_CURRENT_SOURCE_DIR}/IO/Config/EngineConfig.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/IO/Config/PlatformConfig.h
        ${CMAKE_CURRENT_SOURCE_DIR}/IO/Config/ConfigMSWin.h
        ${CMAKE_CURRENT_SOURCE_DIR}/IO/Config/ConfigMSWin.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/IO/Config/ConfigLinux.h
        ${CMAKE_CURRENT_SOURCE_DIR}/IO/Config/ConfigLinux.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/IO/Config/ConfigAndroid.h
        ${CMAKE_CURRENT_SOURCE_DIR}/IO/Config/ConfigAndroid.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/IO/Config/ConfigMacOS.h
        ${CMAKE_CURRENT_SOURCE_DIR}/IO/Config/ConfigMacOS.mm
        ${CMAKE_CURRENT_SOURCE_DIR}/IO/Config/ConfigiOS.h
        ${CMAKE_CURRENT_SOURCE_DIR}/IO/Config/ConfigiOS.mm
        ${CMAKE_CURRENT_SOURCE_DIR}/IO/Config/ConfigEditor.h
        ${CMAKE_CURRENT_SOURCE_DIR}/IO/Config/ConfigEditor.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/IO/GamePackage/PackageHeader.h
        ${CMAKE_CURRENT_SOURCE_DIR}/IO/GamePackage/PackageEntry.h
        ${CMAKE_CURRENT_SOURCE_DIR}/IO/GamePackage/ViewData.h
        ${CMAKE_CURRENT_SOURCE_DIR}/IO/GamePackage/ProjectManifestData.h
        ${CMAKE_CURRENT_SOURCE_DIR}/IO/GamePackage/SceneData.h
        ${CMAKE_CURRENT_SOURCE_DIR}/IO/GamePackage/PrefabData.h
        ${CMAKE_CURRENT_SOURCE_DIR}/Serialize/Serializer.h
        ${CMAKE_CURRENT_SOURCE_DIR}/Serialize/Serializer.cpp
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
