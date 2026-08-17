macro(setup_engine_target TARGET_NAME IS_EDITOR)
    add_library(${TARGET_NAME})

    target_precompile_headers(${TARGET_NAME} PRIVATE
        "EngineIncludes.h"
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

    target_include_directories(${TARGET_NAME}
        PUBLIC
            ${CMAKE_CURRENT_SOURCE_DIR}/..
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}
            pch
            ${CMAKE_CURRENT_SOURCE_DIR}/scene
            ${CMAKE_CURRENT_SOURCE_DIR}/view
            ${CMAKE_CURRENT_SOURCE_DIR}/platforms
    )

    target_sources(${TARGET_NAME} PRIVATE
        # Public
        Engine.h
        Engine.cpp
        EngineIncludes.h
        utils/Fwd.h
        pch/Pch.h
        pch/Pch.cpp

        # Package & Settings
        "package/PackageManager.h" "package/PackageManager.cpp"
        "package/UserSettingsManager.h" "package/UserSettingsManager.cpp"

        # Inputs
        "platforms/input/Input.h" "platforms/input/InputBase.h"

        # Platforms
        "platforms/Platform.h"
        "platforms/PlatformCommon.cpp"

        # Main Loop
        "platforms/mainloop/MainLoop.h" "platforms/mainloop/MainLoopCommon.h" "platforms/mainloop/MainLoopCommon.cpp"
        "platforms/mainloop/MainLoopEditor.h"

        # Native View / Window
        "platforms/window/NativeWindow.h" "platforms/window/WindowCommon.h" "platforms/window/WindowCommon.cpp"
        "view/View.h" "view/View.cpp"
        "view/ViewManager.h" "view/ViewManager.cpp"

        # Helpers
        "platforms/input/InputHelpers.h" "platforms/input/InputBase.cpp"

        # GAPI
        "gapi/IGAPI.h" "gapi/IGAPI.cpp"
        "gapi/GAPI.h"
        "gapi/vulkan/VulkanAPI.h" "gapi/vulkan/VulkanAPI.cpp"
        "gapi/directx12/DirectX12API.h" "gapi/directx12/DirectX12API.cpp"
        "gapi/metal/MetalAPI.h" "gapi/metal/MetalAPI.mm"

        # Scripts & Scenes
        "scene/GameObject.h"
        "scene/Scene.h" "scene/Scene.cpp")

    if(WIN32)
        if(${IS_EDITOR})
            target_sources(${TARGET_NAME} PRIVATE 
                "platforms/PlatformEditor.cpp"
                "platforms/input/InputEditor.h" "platforms/input/InputEditor.cpp"
                "platforms/window/WinMSWindowEditor.h" "platforms/window/WinMSWindowEditor.cpp"
            )
        else()
            target_sources(${TARGET_NAME} PRIVATE 
                "platforms/PlatformWindows.cpp"
                "platforms/input/InputMSWindows.h" "platforms/input/InputMSWindows.cpp"
                "platforms/mainloop/MainLoopMSWin.h" "platforms/mainloop/MainLoopMSWin.cpp"
                "platforms/window/WinMSWindows.h" "platforms/window/WinMSWindows.cpp"
            )
        endif()
    endif()

    if(UNIX AND NOT APPLE AND NOT ANDROID)
        target_sources(${TARGET_NAME} PRIVATE 
            "platforms/PlatformLinux.cpp"
            "platforms/input/InputLinux.h" "platforms/input/InputLinux.cpp"
            "platforms/mainloop/MainLoopLinux.h" "platforms/mainloop/MainLoopLinux.cpp"
            "platforms/window/WinLinux.h" "platforms/window/WinLinux.cpp"
        )
    endif()

    if(ANDROID)
        target_sources(${TARGET_NAME} PRIVATE 
            "platforms/PlatformAndroid.cpp"
            "platforms/input/InputAndroid.h" "platforms/input/InputAndroid.cpp"
            "platforms/mainloop/MainLoopAndroid.h" "platforms/mainloop/MainLoopAndroid.cpp"
            "platforms/window/WinAndroid.h" "platforms/window/WinAndroid.cpp"
        )
    endif()

    if(APPLE)
        if(IOS)
            target_sources(${TARGET_NAME} PRIVATE
                "platforms/input/InputiOS.h" "platforms/input/InputiOS.mm"
                "platforms/PlatformiOS.mm"
                "platforms/mainloop/MainLoopiOS.h" "platforms/mainloop/MainLoopiOS.mm"
                "platforms/window/WiniOS.h" "platforms/window/WiniOS.mm"
            )
        else()
            target_sources(${TARGET_NAME} PRIVATE
                "platforms/input/InputMacOS.h" "platforms/input/InputMacOS.mm"
                "platforms/PlatformMacOS.mm"
                "platforms/mainloop/MainLoopMacOS.h" "platforms/mainloop/MainLoopMacOS.mm"
                "platforms/window/WinMacOS.h" "platforms/window/WinMacOS.mm"
            )
        endif()
    endif()

    target_compile_features(${TARGET_NAME} PUBLIC cxx_std_23)

    if(UNIX AND NOT APPLE AND NOT ANDROID)
        target_link_libraries(${TARGET_NAME} PRIVATE wayland-client)
    endif()

    if(ANDROID)
        find_package(game-activity REQUIRED CONFIG)
    endif()

    if(${IS_EDITOR})
        target_link_libraries(${TARGET_NAME}
            PUBLIC
                core_lib_editor
                logger_lib_editor
        )
    else()
        target_link_libraries(${TARGET_NAME}
            PUBLIC
                core_lib
                logger_lib
        )
    endif()

    target_compile_definitions(${TARGET_NAME} PRIVATE Z_TARGET_NAME="${TARGET_NAME}")
    set_source_files_properties(pch/Pch.cpp PROPERTIES SKIP_PRECOMPILE_HEADERS ON)
    if(${IS_EDITOR})
        target_compile_definitions(${TARGET_NAME} PUBLIC Z_EDITOR=1 EDITORDLL_EXPORTS)
    endif()
endmacro()

setup_engine_target(engine_lib FALSE)
setup_engine_target(engine_lib_editor TRUE)
