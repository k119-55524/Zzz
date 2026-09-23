macro(setup_core_target TARGET_NAME IS_EDITOR)
    add_library(${TARGET_NAME} STATIC)

    target_include_directories(${TARGET_NAME} PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/..
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/pch
        ${CMAKE_CURRENT_SOURCE_DIR}/logger
    )

    target_sources(${TARGET_NAME} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/CoreIncludes.h
        ${CMAKE_CURRENT_SOURCE_DIR}/Core.h
        ${CMAKE_CURRENT_SOURCE_DIR}/utils/NativeAppData.h
        ${CMAKE_CURRENT_SOURCE_DIR}/utils/Export.h
        ${CMAKE_CURRENT_SOURCE_DIR}/constants/PackagesConstants.h
        ${CMAKE_CURRENT_SOURCE_DIR}/constants/DisplayConstants.h
        ${CMAKE_CURRENT_SOURCE_DIR}/constants/GAPIConstants.h
        ${CMAKE_CURRENT_SOURCE_DIR}/constants/NetworkConstants.h
        ${CMAKE_CURRENT_SOURCE_DIR}/constants/LogCategoryConstants.h
        ${CMAKE_CURRENT_SOURCE_DIR}/constants/platforms/PlatformConstants.h
        ${CMAKE_CURRENT_SOURCE_DIR}/constants/platforms/MSWinConstants.h
        ${CMAKE_CURRENT_SOURCE_DIR}/constants/platforms/LinuxConstants.h
        ${CMAKE_CURRENT_SOURCE_DIR}/constants/platforms/MacOSConstants.h
        ${CMAKE_CURRENT_SOURCE_DIR}/constants/platforms/AndroidConstants.h
        ${CMAKE_CURRENT_SOURCE_DIR}/constants/platforms/iOSConstants.h
        ${CMAKE_CURRENT_SOURCE_DIR}/utils/Defines.h
        ${CMAKE_CURRENT_SOURCE_DIR}/utils/Converters.h
        ${CMAKE_CURRENT_SOURCE_DIR}/utils/Ensure.h
        ${CMAKE_CURRENT_SOURCE_DIR}/utils/Guid.h
        ${CMAKE_CURRENT_SOURCE_DIR}/utils/Guid.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utils/Macroses.h
        ${CMAKE_CURRENT_SOURCE_DIR}/utils/MemoryUtils.h
        ${CMAKE_CURRENT_SOURCE_DIR}/utils/SafeMath.h
        ${CMAKE_CURRENT_SOURCE_DIR}/utils/ThrowWrappers.h
        ${CMAKE_CURRENT_SOURCE_DIR}/utils/ThrowWrappers.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utils/FixedLengthString.h
        ${CMAKE_CURRENT_SOURCE_DIR}/utils/Version.h
        ${CMAKE_CURRENT_SOURCE_DIR}/utils/LogCategory.h
        ${CMAKE_CURRENT_SOURCE_DIR}/utils/ToStringHelpers.h
        ${CMAKE_CURRENT_SOURCE_DIR}/utils/ThreadUtils.h
        ${CMAKE_CURRENT_SOURCE_DIR}/utils/ThreadUtils.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utils/platforms/DX12ToStringHelpers.h
        ${CMAKE_CURRENT_SOURCE_DIR}/utils/platforms/VKToStringHelpers.h
        ${CMAKE_CURRENT_SOURCE_DIR}/utils/platforms/MetalToStringHelpers.h
        ${CMAKE_CURRENT_SOURCE_DIR}/utils/macros/LogMacros.h
        ${CMAKE_CURRENT_SOURCE_DIR}/logger/LoggerIncludes.h
        ${CMAKE_CURRENT_SOURCE_DIR}/logger/logger.h
        ${CMAKE_CURRENT_SOURCE_DIR}/logger/logger.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/logger/private/log_entry.h
        ${CMAKE_CURRENT_SOURCE_DIR}/logger/private/log_entry.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/logger/private/IBroadcaster.h
        ${CMAKE_CURRENT_SOURCE_DIR}/logger/private/CallbackBroadcaster.h
        ${CMAKE_CURRENT_SOURCE_DIR}/logger/private/ConsoleBroadcaster.h
        ${CMAKE_CURRENT_SOURCE_DIR}/logger/private/NetworkBroadcaster.h
        ${CMAKE_CURRENT_SOURCE_DIR}/logger/private/NetworkBroadcaster.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/utils/macros/MiscMacros.h
        ${CMAKE_CURRENT_SOURCE_DIR}/headers/Apple.h
        ${CMAKE_CURRENT_SOURCE_DIR}/headers/platforms/MSWin.h
        ${CMAKE_CURRENT_SOURCE_DIR}/headers/Linux.h
        ${CMAKE_CURRENT_SOURCE_DIR}/headers/Android.h
        ${CMAKE_CURRENT_SOURCE_DIR}/headers/platforms/DirectX12.h
        ${CMAKE_CURRENT_SOURCE_DIR}/headers/platforms/Vulkan.h
        ${CMAKE_CURRENT_SOURCE_DIR}/headers/platforms/Metal.h
        ${CMAKE_CURRENT_SOURCE_DIR}/enums/eInitState.h
        ${CMAKE_CURRENT_SOURCE_DIR}/enums/eWindowState.h
        ${CMAKE_CURRENT_SOURCE_DIR}/time/Time.h
        ${CMAKE_CURRENT_SOURCE_DIR}/time/Time.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/events/Event.h
        ${CMAKE_CURRENT_SOURCE_DIR}/events/EventBus.h
        ${CMAKE_CURRENT_SOURCE_DIR}/events/OneShotEvent.h
        ${CMAKE_CURRENT_SOURCE_DIR}/userscripts/ScriptStorage.h
        ${CMAKE_CURRENT_SOURCE_DIR}/userscripts/ScriptStorage.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/userscripts/ScriptFactory.h
        ${CMAKE_CURRENT_SOURCE_DIR}/userscripts/ScriptFactory.cpp
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
        ${CMAKE_CURRENT_SOURCE_DIR}/utils/Alignment.h
        ${CMAKE_CURRENT_SOURCE_DIR}/enums/eResourceType.h
        ${CMAKE_CURRENT_SOURCE_DIR}/enums/ePixelFormat.h
        ${CMAKE_CURRENT_SOURCE_DIR}/enums/eIndexFormat.h
        ${CMAKE_CURRENT_SOURCE_DIR}/enums/eVertexSemantic.h
        ${CMAKE_CURRENT_SOURCE_DIR}/enums/eFileLocation.h
        ${CMAKE_CURRENT_SOURCE_DIR}/enums/eLogMessageType.h
        ${CMAKE_CURRENT_SOURCE_DIR}/enums/eWinResize.h
        ${CMAKE_CURRENT_SOURCE_DIR}/enums/eGAPIType.h
        ${CMAKE_CURRENT_SOURCE_DIR}/enums/eLayerType.h
        ${CMAKE_CURRENT_SOURCE_DIR}/enums/eSpatialStorageType.h
        ${CMAKE_CURRENT_SOURCE_DIR}/enums/ePackage.h
        ${CMAKE_CURRENT_SOURCE_DIR}/enums/eTargetPlatform.h
        ${CMAKE_CURRENT_SOURCE_DIR}/enums/platforms/ConverterGAPITypes.h
        ${CMAKE_CURRENT_SOURCE_DIR}/enums/platforms/ConverterMSWinTypes.h
        ${CMAKE_CURRENT_SOURCE_DIR}/enums/platforms/ConverterAndroidTypes.h
        ${CMAKE_CURRENT_SOURCE_DIR}/enums/platforms/ConverteriOSTypes.h
        ${CMAKE_CURRENT_SOURCE_DIR}/enums/platforms/ConverterLinuxTypes.h
        ${CMAKE_CURRENT_SOURCE_DIR}/enums/platforms/ConverterMacOSTypes.h
        ${CMAKE_CURRENT_SOURCE_DIR}/enums/platforms/eAndroidEnums.h
        ${CMAKE_CURRENT_SOURCE_DIR}/enums/platforms/eLinuxEnums.h
        ${CMAKE_CURRENT_SOURCE_DIR}/enums/platforms/eMSWinEnums.h
        ${CMAKE_CURRENT_SOURCE_DIR}/enums/platforms/eMacOSEnums.h
        ${CMAKE_CURRENT_SOURCE_DIR}/enums/platforms/eiOSEnums.h
        ${CMAKE_CURRENT_SOURCE_DIR}/templates/DoubleBufferedVector.h
        ${CMAKE_CURRENT_SOURCE_DIR}/templates/CallbackQueue.h
        ${CMAKE_CURRENT_SOURCE_DIR}/templates/CountdownTrigger.h
        ${CMAKE_CURRENT_SOURCE_DIR}/templates/ThreadPool.h
        ${CMAKE_CURRENT_SOURCE_DIR}/templates/AsyncInitTracker.h
        ${CMAKE_CURRENT_SOURCE_DIR}/containers/BitTreeTracker.h
        ${CMAKE_CURRENT_SOURCE_DIR}/containers/BitTreeTracker.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/scene/SceneTransitionParams.h
        ${CMAKE_CURRENT_SOURCE_DIR}/hardware/CpuInfo.h
        ${CMAKE_CURRENT_SOURCE_DIR}/hardware/CpuTopology.h
        ${CMAKE_CURRENT_SOURCE_DIR}/hardware/GpuInfo.h
        ${CMAKE_CURRENT_SOURCE_DIR}/hardware/RamInfo.h
        ${CMAKE_CURRENT_SOURCE_DIR}/hardware/MotherboardInfo.h
        ${CMAKE_CURRENT_SOURCE_DIR}/hardware/MonitorInfo.h
        ${CMAKE_CURRENT_SOURCE_DIR}/hardware/StorageInfo.h
        ${CMAKE_CURRENT_SOURCE_DIR}/hardware/NetworkAdapterInfo.h
        ${CMAKE_CURRENT_SOURCE_DIR}/hardware/HardwareState.h
        ${CMAKE_CURRENT_SOURCE_DIR}/io/DatFileHeader.h
        ${CMAKE_CURRENT_SOURCE_DIR}/io/Path.h
        ${CMAKE_CURRENT_SOURCE_DIR}/io/Path.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/io/FileSystem.h
        ${CMAKE_CURRENT_SOURCE_DIR}/io/FileSystemBase.h
        ${CMAKE_CURRENT_SOURCE_DIR}/io/FileSystemBase.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/io/platforms/FileSystemDesktop.h
        ${CMAKE_CURRENT_SOURCE_DIR}/io/platforms/FileSystemDesktop.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/io/platforms/FileSystemAndroid.h
        ${CMAKE_CURRENT_SOURCE_DIR}/io/platforms/FileSystemAndroid.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/io/platforms/FileSystemiOS.h
        ${CMAKE_CURRENT_SOURCE_DIR}/io/platforms/FileSystemiOS.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/io/package/PackageEntry.h
        ${CMAKE_CURRENT_SOURCE_DIR}/io/package/DataAssetsManager.h
        ${CMAKE_CURRENT_SOURCE_DIR}/io/package/DataAssetsManager.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/io/package/ProjectManifestData.h
        ${CMAKE_CURRENT_SOURCE_DIR}/io/package/assets/MeshData.h
        ${CMAKE_CURRENT_SOURCE_DIR}/io/package/assets/MeshData.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/io/package/assets/MaterialData.h
        ${CMAKE_CURRENT_SOURCE_DIR}/io/package/assets/MaterialData.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/io/package/assets/ShaderData.h
        ${CMAKE_CURRENT_SOURCE_DIR}/io/package/assets/ShaderData.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/io/package/assets/PrefabData.h
        ${CMAKE_CURRENT_SOURCE_DIR}/io/package/assets/GameObjectData.h
        ${CMAKE_CURRENT_SOURCE_DIR}/io/package/assets/GameObjectData.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/io/package/views/PrimaryViewData.h
        ${CMAKE_CURRENT_SOURCE_DIR}/io/package/views/ChildViewData.h
        ${CMAKE_CURRENT_SOURCE_DIR}/io/package/views/IndependentViewData.h
        ${CMAKE_CURRENT_SOURCE_DIR}/io/package/views/ViewConfigData.h
        ${CMAKE_CURRENT_SOURCE_DIR}/io/package/views/ViewUserData.h
        ${CMAKE_CURRENT_SOURCE_DIR}/io/package/views/PrimaryViewUserData.h
        ${CMAKE_CURRENT_SOURCE_DIR}/io/package/scene/SceneData.h
        ${CMAKE_CURRENT_SOURCE_DIR}/io/package/scene/LayerData.h
        ${CMAKE_CURRENT_SOURCE_DIR}/io/package/scene/LayerData.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/io/package/platforms/project/ProjectPlatformConfig.h
        ${CMAKE_CURRENT_SOURCE_DIR}/io/package/platforms/project/ProjectPlatformDataMSWin.h
        ${CMAKE_CURRENT_SOURCE_DIR}/io/package/platforms/project/ProjectPlatformDataAndroid.h
        ${CMAKE_CURRENT_SOURCE_DIR}/io/package/platforms/project/ProjectPlatformDataiOS.h
        ${CMAKE_CURRENT_SOURCE_DIR}/io/package/platforms/project/ProjectPlatformDataLinux.h
        ${CMAKE_CURRENT_SOURCE_DIR}/io/package/platforms/project/ProjectPlatformDataMacOS.h
        ${CMAKE_CURRENT_SOURCE_DIR}/io/package/platforms/start_view/ViewPlatformConfig.h
        ${CMAKE_CURRENT_SOURCE_DIR}/io/package/platforms/start_view/ViewDataMSWin.h
        ${CMAKE_CURRENT_SOURCE_DIR}/io/package/platforms/start_view/ViewDataAndroid.h
        ${CMAKE_CURRENT_SOURCE_DIR}/io/package/platforms/start_view/ViewDataiOS.h
        ${CMAKE_CURRENT_SOURCE_DIR}/io/package/platforms/start_view/ViewDataLinux.h
        ${CMAKE_CURRENT_SOURCE_DIR}/io/package/platforms/start_view/ViewDataMacOS.h
        ${CMAKE_CURRENT_SOURCE_DIR}/render/clear_config/ClearConfig.h
        ${CMAKE_CURRENT_SOURCE_DIR}/render/clear_config/DepthBufferClearConfig.h
        ${CMAKE_CURRENT_SOURCE_DIR}/render/clear_config/SurfaceClearConfig.h
        ${CMAKE_CURRENT_SOURCE_DIR}/serialize/Serializer.h
        ${CMAKE_CURRENT_SOURCE_DIR}/serialize/Serializer.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/pch/pch.h
        ${CMAKE_CURRENT_SOURCE_DIR}/pch/pch.cpp
    )

    set_source_files_properties("${CMAKE_CURRENT_SOURCE_DIR}/pch/pch.cpp" PROPERTIES SKIP_PRECOMPILE_HEADERS ON)

    target_precompile_headers(${TARGET_NAME} PRIVATE
        "pch/pch.h"
    )

    if(UNIX AND NOT APPLE AND NOT ANDROID)
        target_sources(${TARGET_NAME} PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/specific/linux_wayland/Xdg-shell-client-protocol.h
            ${CMAKE_CURRENT_SOURCE_DIR}/specific/linux_wayland/Xdg-shell-protocol.cpp
        )
        set_source_files_properties(
            "${CMAKE_CURRENT_SOURCE_DIR}/specific/linux_wayland/Xdg-shell-protocol.cpp"
            PROPERTIES COMPILE_OPTIONS "-Wno-attributes"
        )
    endif()

    if(APPLE)
        target_sources(${TARGET_NAME} PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/io/apple/Path_Apple.mm
        )
    endif()

    target_compile_features(${TARGET_NAME} PUBLIC cxx_std_23)
    target_compile_definitions(${TARGET_NAME} PRIVATE Z_TARGET_NAME="${TARGET_NAME}")

    if(${IS_EDITOR})
        target_compile_definitions(${TARGET_NAME} PUBLIC Z_EDITOR=1 EDITORDLL_EXPORTS)
        target_link_libraries(${TARGET_NAME} PUBLIC math_editor)
    else()
        target_link_libraries(${TARGET_NAME} PUBLIC math)
    endif()

    if(WIN32)
        target_link_libraries(${TARGET_NAME} PRIVATE User32 Ws2_32)
    endif()

    if(ANDROID)
        target_link_libraries(${TARGET_NAME} PRIVATE log)
    endif()

    if(TARGET Microsoft::WinPixEventRuntime)
        target_link_libraries(${TARGET_NAME} PUBLIC Microsoft::WinPixEventRuntime)
    endif()

    if(TARGET Microsoft::DXC)
        target_link_libraries(${TARGET_NAME} PUBLIC Microsoft::DXC)
    endif()

    if((WIN32 AND Z_FORCE_VULKAN) OR LINUX OR ANDROID)
        find_package(Vulkan REQUIRED)
        target_link_libraries(${TARGET_NAME} PUBLIC Vulkan::Vulkan)
    endif()

    if(ANDROID)
        find_package(game-activity REQUIRED CONFIG)
        target_link_libraries(${TARGET_NAME} PUBLIC game-activity::game-activity)
    endif()
endmacro()

setup_core_target(core_lib FALSE)
setup_core_target(core_lib_editor TRUE)

add_library(logger_lib ALIAS core_lib)
if(WIN32)
    add_library(logger_lib_editor ALIAS core_lib_editor)
endif()
