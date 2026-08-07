macro(setup_core_target TARGET_NAME IS_EDITOR)
    add_library(${TARGET_NAME} STATIC)

    target_include_directories(${TARGET_NAME} PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/..
        ${CMAKE_CURRENT_SOURCE_DIR}/userscripts
        ${CMAKE_CURRENT_SOURCE_DIR}/userscripts/base_script
    )

    target_sources(${TARGET_NAME} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/pch.h
        ${CMAKE_CURRENT_SOURCE_DIR}/pch.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/CoreIncludes.h
        ${CMAKE_CURRENT_SOURCE_DIR}/core.h
        ${CMAKE_CURRENT_SOURCE_DIR}/utils/Types.h
        ${CMAKE_CURRENT_SOURCE_DIR}/utils/NativeAppData.h
        ${CMAKE_CURRENT_SOURCE_DIR}/utils/Export.h
        ${CMAKE_CURRENT_SOURCE_DIR}/utils/Constants.h
        ${CMAKE_CURRENT_SOURCE_DIR}/utils/Defines.h
        ${CMAKE_CURRENT_SOURCE_DIR}/utils/Ensure.h
        ${CMAKE_CURRENT_SOURCE_DIR}/utils/Guid.h
        ${CMAKE_CURRENT_SOURCE_DIR}/utils/Macroses.h
        ${CMAKE_CURRENT_SOURCE_DIR}/utils/MemoryUtils.h
        ${CMAKE_CURRENT_SOURCE_DIR}/utils/ThrowWrappers.h
        ${CMAKE_CURRENT_SOURCE_DIR}/utils/ThrowWrappers.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utils/Version.h
        ${CMAKE_CURRENT_SOURCE_DIR}/headers/Apple.h
        ${CMAKE_CURRENT_SOURCE_DIR}/headers/MSWin.h
        ${CMAKE_CURRENT_SOURCE_DIR}/headers/Linux.h
        ${CMAKE_CURRENT_SOURCE_DIR}/headers/Android.h
        ${CMAKE_CURRENT_SOURCE_DIR}/headers/DirectX12.h
        ${CMAKE_CURRENT_SOURCE_DIR}/headers/Vulkan.h
        ${CMAKE_CURRENT_SOURCE_DIR}/headers/Metal.h
        ${CMAKE_CURRENT_SOURCE_DIR}/enums/eInitState.h
        ${CMAKE_CURRENT_SOURCE_DIR}/time/Time.h
        ${CMAKE_CURRENT_SOURCE_DIR}/time/Time.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/events/Event.h
        ${CMAKE_CURRENT_SOURCE_DIR}/events/EventBus.h
        ${CMAKE_CURRENT_SOURCE_DIR}/userscripts/ScriptRegistry.h
        ${CMAKE_CURRENT_SOURCE_DIR}/userscripts/ScriptRegistry.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/userscripts/base_script/BaseScript.h
        ${CMAKE_CURRENT_SOURCE_DIR}/userscripts/base_script/Script.h
        ${CMAKE_CURRENT_SOURCE_DIR}/userscripts/base_script/Script.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/userscripts/base_script/GameScript.h
        ${CMAKE_CURRENT_SOURCE_DIR}/userscripts/base_script/GameScript.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/userscripts/base_script/SceneScript.h
        ${CMAKE_CURRENT_SOURCE_DIR}/userscripts/base_script/SceneScript.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/userscripts/base_script/ViewScript.h
        ${CMAKE_CURRENT_SOURCE_DIR}/userscripts/base_script/ViewScript.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/enums/eLogMessageType.h
        ${CMAKE_CURRENT_SOURCE_DIR}/enums/eWinResize.h
        ${CMAKE_CURRENT_SOURCE_DIR}/enums/eGAPIType.h
        ${CMAKE_CURRENT_SOURCE_DIR}/Enums/ePackage.h
        ${CMAKE_CURRENT_SOURCE_DIR}/Enums/eTargetPlatform.h
        ${CMAKE_CURRENT_SOURCE_DIR}/Enums/eEnumToString.h
        ${CMAKE_CURRENT_SOURCE_DIR}/Enums/platforms/ConverterMSWinTypes.h
        ${CMAKE_CURRENT_SOURCE_DIR}/Enums/platforms/ConverterAndroidTypes.h
        ${CMAKE_CURRENT_SOURCE_DIR}/Enums/platforms/ConverteriOSTypes.h
        ${CMAKE_CURRENT_SOURCE_DIR}/Enums/platforms/ConverterLinuxTypes.h
        ${CMAKE_CURRENT_SOURCE_DIR}/Enums/platforms/ConverterMacOSTypes.h
        ${CMAKE_CURRENT_SOURCE_DIR}/templates/DoubleBufferedVector.h
        ${CMAKE_CURRENT_SOURCE_DIR}/templates/Size2D.h
        ${CMAKE_CURRENT_SOURCE_DIR}/IO/FileHeader.h
        ${CMAKE_CURRENT_SOURCE_DIR}/IO/Path.h
        ${CMAKE_CURRENT_SOURCE_DIR}/IO/Path.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/IO/config/PlatformConfig.h
        ${CMAKE_CURRENT_SOURCE_DIR}/IO/config/platforms/ConfigMSWin.h
        ${CMAKE_CURRENT_SOURCE_DIR}/IO/config/platforms/ConfigMSWin.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/IO/config/platforms/ConfigLinux.h
        ${CMAKE_CURRENT_SOURCE_DIR}/IO/config/platforms/ConfigLinux.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/IO/config/platforms/ConfigAndroid.h
        ${CMAKE_CURRENT_SOURCE_DIR}/IO/config/platforms/ConfigAndroid.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/IO/config/platforms/ConfigMacOS.h
        ${CMAKE_CURRENT_SOURCE_DIR}/IO/config/platforms/ConfigMacOS.mm
        ${CMAKE_CURRENT_SOURCE_DIR}/IO/config/platforms/ConfigiOS.h
        ${CMAKE_CURRENT_SOURCE_DIR}/IO/config/platforms/ConfigiOS.mm
        ${CMAKE_CURRENT_SOURCE_DIR}/IO/config/platforms/ConfigEditor.h
        ${CMAKE_CURRENT_SOURCE_DIR}/IO/config/platforms/ConfigEditor.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/IO/package/PackageHeader.h
        ${CMAKE_CURRENT_SOURCE_DIR}/IO/package/PackageEntry.h
        ${CMAKE_CURRENT_SOURCE_DIR}/IO/package/StartViewData.h
        ${CMAKE_CURRENT_SOURCE_DIR}/IO/package/platforms/StartViewPlatformConfig.h
        ${CMAKE_CURRENT_SOURCE_DIR}/IO/package/platforms/StartViewDataMSWin.h
        ${CMAKE_CURRENT_SOURCE_DIR}/IO/package/platforms/StartViewDataAndroid.h
        ${CMAKE_CURRENT_SOURCE_DIR}/IO/package/platforms/StartViewDataiOS.h
        ${CMAKE_CURRENT_SOURCE_DIR}/IO/package/platforms/StartViewDataLinux.h
        ${CMAKE_CURRENT_SOURCE_DIR}/IO/package/platforms/StartViewDataMacOS.h
        ${CMAKE_CURRENT_SOURCE_DIR}/IO/package/StartViewUserData.h
        ${CMAKE_CURRENT_SOURCE_DIR}/IO/package/ViewData.h
        ${CMAKE_CURRENT_SOURCE_DIR}/IO/package/ProjectManifestData.h
        ${CMAKE_CURRENT_SOURCE_DIR}/IO/package/SceneData.h
        ${CMAKE_CURRENT_SOURCE_DIR}/IO/package/PrefabData.h
        ${CMAKE_CURRENT_SOURCE_DIR}/Serialize/Serializer.h
        ${CMAKE_CURRENT_SOURCE_DIR}/Serialize/Serializer.cpp
    )

    target_precompile_headers(${TARGET_NAME} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/pch.h
    )

    if(APPLE)
        target_sources(${TARGET_NAME} PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/IO/apple/Path_Apple.mm
        )
    endif()

    target_compile_features(${TARGET_NAME} PUBLIC cxx_std_23)
    target_compile_definitions(${TARGET_NAME} PRIVATE Z_TARGET_NAME="${TARGET_NAME}")
    set_source_files_properties(pch.cpp PROPERTIES SKIP_PRECOMPILE_HEADERS ON)

    if(${IS_EDITOR})
        target_compile_definitions(${TARGET_NAME} PUBLIC Z_EDITOR=1 EDITORDLL_EXPORTS)
    endif()

    export_to_editor_sdk(${TARGET_NAME} "${CMAKE_CURRENT_SOURCE_DIR}" "core")
endmacro()

setup_core_target(core_lib FALSE)
setup_core_target(core_lib_editor TRUE)
