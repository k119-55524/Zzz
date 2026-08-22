macro(setup_core_target TARGET_NAME IS_EDITOR)
    add_library(${TARGET_NAME} STATIC)

    target_include_directories(${TARGET_NAME} PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/..
    )

    target_sources(${TARGET_NAME} PRIVATE
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
        ${CMAKE_CURRENT_SOURCE_DIR}/hardware/CpuInfo.h
        ${CMAKE_CURRENT_SOURCE_DIR}/hardware/GpuInfo.h
        ${CMAKE_CURRENT_SOURCE_DIR}/hardware/RamInfo.h
        ${CMAKE_CURRENT_SOURCE_DIR}/hardware/MotherboardInfo.h
        ${CMAKE_CURRENT_SOURCE_DIR}/hardware/MonitorInfo.h
        ${CMAKE_CURRENT_SOURCE_DIR}/hardware/StorageInfo.h
        ${CMAKE_CURRENT_SOURCE_DIR}/hardware/NetworkAdapterInfo.h
        ${CMAKE_CURRENT_SOURCE_DIR}/hardware/HardwareState.h
        ${CMAKE_CURRENT_SOURCE_DIR}/IO/FileHeader.h
        ${CMAKE_CURRENT_SOURCE_DIR}/IO/Path.h
        ${CMAKE_CURRENT_SOURCE_DIR}/IO/Path.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/IO/package/PackageHeader.h
        ${CMAKE_CURRENT_SOURCE_DIR}/IO/package/PackageEntry.h
        ${CMAKE_CURRENT_SOURCE_DIR}/IO/package/PrimaryViewData.h
        ${CMAKE_CURRENT_SOURCE_DIR}/IO/package/platforms/project/ProjectPlatformConfig.h
        ${CMAKE_CURRENT_SOURCE_DIR}/IO/package/platforms/project/ProjectPlatformDataMSWin.h
        ${CMAKE_CURRENT_SOURCE_DIR}/IO/package/platforms/project/ProjectPlatformDataAndroid.h
        ${CMAKE_CURRENT_SOURCE_DIR}/IO/package/platforms/project/ProjectPlatformDataiOS.h
        ${CMAKE_CURRENT_SOURCE_DIR}/IO/package/platforms/project/ProjectPlatformDataLinux.h
        ${CMAKE_CURRENT_SOURCE_DIR}/IO/package/platforms/project/ProjectPlatformDataMacOS.h
        ${CMAKE_CURRENT_SOURCE_DIR}/IO/package/platforms/start_view/ViewPlatformConfig.h
        ${CMAKE_CURRENT_SOURCE_DIR}/IO/package/platforms/start_view/ViewDataMSWin.h
        ${CMAKE_CURRENT_SOURCE_DIR}/IO/package/platforms/start_view/ViewDataAndroid.h
        ${CMAKE_CURRENT_SOURCE_DIR}/IO/package/platforms/start_view/ViewDataiOS.h
        ${CMAKE_CURRENT_SOURCE_DIR}/IO/package/platforms/start_view/ViewDataLinux.h
        ${CMAKE_CURRENT_SOURCE_DIR}/IO/package/platforms/start_view/ViewDataMacOS.h
        ${CMAKE_CURRENT_SOURCE_DIR}/IO/package/PrimaryViewUserData.h
        ${CMAKE_CURRENT_SOURCE_DIR}/IO/package/ViewUserData.h
        ${CMAKE_CURRENT_SOURCE_DIR}/IO/package/ViewData.h
        ${CMAKE_CURRENT_SOURCE_DIR}/IO/package/ProjectManifestData.h
        ${CMAKE_CURRENT_SOURCE_DIR}/IO/package/SceneData.h
        ${CMAKE_CURRENT_SOURCE_DIR}/IO/package/PrefabData.h
        ${CMAKE_CURRENT_SOURCE_DIR}/Serialize/Serializer.h
        ${CMAKE_CURRENT_SOURCE_DIR}/Serialize/Serializer.cpp
    )

    target_precompile_headers(${TARGET_NAME} PRIVATE
        "Core.h"
        <cstdint>
        <cstddef>
        <cstring>
        <string>
        <string_view>
        <vector>
        <array>
        <list>
        <map>
        <unordered_map>
        <set>
        <unordered_set>
        <memory>
        <utility>
        <algorithm>
        <functional>
        <type_traits>
        <chrono>
        <thread>
        <mutex>
        <atomic>
        <filesystem>
        <fstream>
        <sstream>
        <iostream>
        <variant>
        <optional>
        <stdexcept>
        <system_error>
        <format>
        <concepts>
        $<$<BOOL:${WIN32}>:<winsock2.h>>
        $<$<BOOL:${WIN32}>:<ws2tcpip.h>>
        $<$<BOOL:${WIN32}>:<windows.h>>
        $<$<BOOL:${WIN32}>:<d3d12.h>>
        $<$<BOOL:${WIN32}>:<dxcapi.h>>
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
            ${CMAKE_CURRENT_SOURCE_DIR}/IO/apple/Path_Apple.mm
        )
    endif()

    target_compile_features(${TARGET_NAME} PUBLIC cxx_std_23)
    target_compile_definitions(${TARGET_NAME} PRIVATE Z_TARGET_NAME="${TARGET_NAME}")

    if(${IS_EDITOR})
        target_compile_definitions(${TARGET_NAME} PUBLIC Z_EDITOR=1 EDITORDLL_EXPORTS)
        target_link_libraries(${TARGET_NAME} PUBLIC logger_lib_editor math_editor)
    else()
        target_link_libraries(${TARGET_NAME} PUBLIC logger_lib math)
    endif()

    if(TARGET Microsoft::WinPixEventRuntime)
        target_link_libraries(${TARGET_NAME} PUBLIC Microsoft::WinPixEventRuntime)
    endif()

    if(TARGET Microsoft::DXC)
        target_link_libraries(${TARGET_NAME} PUBLIC Microsoft::DXC)
    endif()

    if(NOT WIN32 OR Z_FORCE_VULKAN)
        find_package(Vulkan REQUIRED)
        target_link_libraries(${TARGET_NAME} PUBLIC Vulkan::Vulkan)
        target_compile_definitions(${TARGET_NAME} PUBLIC Z_HAS_VULKAN_SDK=1)
    endif()

    if(ANDROID)
        find_package(game-activity REQUIRED CONFIG)
        target_link_libraries(${TARGET_NAME} PUBLIC game-activity::game-activity)
    endif()
endmacro()

setup_core_target(core_lib FALSE)
setup_core_target(core_lib_editor TRUE)
