macro(setup_engine_target TARGET_NAME IS_EDITOR)
    add_library(${TARGET_NAME})

    target_precompile_headers(${TARGET_NAME} PRIVATE
        pch/pch.h
    )

    target_include_directories(${TARGET_NAME}
        PUBLIC
            ${CMAKE_CURRENT_SOURCE_DIR}
            ${CMAKE_CURRENT_SOURCE_DIR}/public/core/scene
            ${CMAKE_CURRENT_SOURCE_DIR}/public/core/userscripts
            ${CMAKE_CURRENT_SOURCE_DIR}/public/core/userscripts/base_script
        PRIVATE
            pch
            ${CMAKE_CURRENT_SOURCE_DIR}/private/core
            ${CMAKE_CURRENT_SOURCE_DIR}/private/platforms
    )

    target_sources(${TARGET_NAME} PRIVATE
        # Public
        engine.h
        engine.cpp
        header.h
        NativeAppData.h
        pch/pch.h
        pch/pch.cpp

        # Headers
        "headers/Apple.h"
        "headers/MSWin.h"
        "headers/Linux.h"
        "headers/Android.h"
        "headers/enums.h"

        # Core
        "private/platforms/config/ConfigManager.h" "private/platforms/config/ConfigManager.cpp"
        "private/platforms/package/PackageManager.h" "private/platforms/package/PackageManager.cpp"
        "private/platforms/config/EngineConfig.h" "private/platforms/config/EngineConfig.cpp"
        "private/platforms/config/PlatformConfig.h"
        "private/core/io/Path.h" "private/core/io/Path.cpp"
        "public/core/EngineTime.h" "public/core/EngineTime.cpp"

        # Inputs
        "private/platforms/input/Input.h" "private/platforms/input/InputBase.h"

        # Platforms
        "private/platforms/Platform.h"
        "private/platforms/Platform_Common.cpp"

        # Main Loop
        "private/platforms/main_loop/MainLoop.h" "private/platforms/main_loop/MainLoop_Common.h" "private/platforms/main_loop/MainLoop_Common.cpp"
        "private/platforms/main_loop/MainLoop_Editor.h"

        # Native View / Window
        "private/platforms/window/Window.h" "private/platforms/window/Window_Common.h" "private/platforms/window/Window_Common.cpp"
        "private/core/view/View.h" "private/core/view/View.cpp"
        "private/core/view/ViewManager.h" "private/core/view/ViewManager.cpp"
        "public/core/utils/ScreenResolution.h"

        # Wayland (Linux)
        "private/core/specific/linux_wayland/xdg-shell-client-protocol.h"
        "private/core/specific/linux_wayland/xdg-shell-protocol.cpp"
      "private/core/utils/converters.h" "private/platforms/input/InputHelpers.h" "private/platforms/input/InputBase.cpp"

        # Scripts & Scenes
        "public/core/scene/GameObject.h"
        "public/core/scene/Scene.h" "private/core/scene/Scene.cpp"
        "public/core/events/EventBus.h"
        "public/core/userscripts/ScriptRegistry.h" "public/core/userscripts/ScriptRegistry.cpp"
        "public/core/userscripts/base_script/Script.h"
        "public/core/userscripts/base_script/Script.cpp"
        "public/core/userscripts/base_script/GameScript.h"
        "public/core/userscripts/base_script/GameScript.cpp"
        "public/core/userscripts/base_script/SceneScript.h"
        "public/core/userscripts/base_script/SceneScript.cpp"
        "public/core/userscripts/base_script/ViewScript.h"
        "public/core/userscripts/base_script/ViewScript.cpp")

    if(WIN32)
        if(${IS_EDITOR})
            target_sources(${TARGET_NAME} PRIVATE 
                "private/platforms/Platform_Editor.cpp"
                "private/platforms/config/ConfigEditor.h" "private/platforms/config/ConfigEditor.cpp"
                "private/platforms/input/InputEditor.h" "private/platforms/input/InputEditor.cpp"
                "private/platforms/window/WinMSWindowEditor.h" "private/platforms/window/WinMSWindowEditor.cpp"
            )
        else()
            target_sources(${TARGET_NAME} PRIVATE 
                "private/platforms/Platform_Windows.cpp"
                "private/platforms/config/ConfigMSWin.h" "private/platforms/config/ConfigMSWin.cpp"
                "private/platforms/input/InputMSWindows.h" "private/platforms/input/InputMSWindows.cpp"
                "private/platforms/main_loop/MainLoop_MSWin.h" "private/platforms/main_loop/MainLoop_MSWin.cpp"
                "private/platforms/window/WinMSWindows.h" "private/platforms/window/WinMSWindows.cpp"
            )
        endif()
    endif()

    if(UNIX AND NOT APPLE AND NOT ANDROID)
        target_sources(${TARGET_NAME} PRIVATE 
            "private/platforms/Platform_Linux.cpp"
            "private/platforms/config/ConfigLinux.h" "private/platforms/config/ConfigLinux.cpp"
            "private/platforms/input/InputLinux.h" "private/platforms/input/InputLinux.cpp"
            "private/platforms/main_loop/MainLoop_Linux.h" "private/platforms/main_loop/MainLoop_Linux.cpp"
            "private/platforms/window/WinLinux.h" "private/platforms/window/WinLinux.cpp"
        )
    endif()

    if(ANDROID)
        target_sources(${TARGET_NAME} PRIVATE 
            "private/platforms/Platform_Android.cpp"
            "private/platforms/config/ConfigAndroid.h" "private/platforms/config/ConfigAndroid.cpp"
            "private/platforms/input/InputAndroid.h" "private/platforms/input/InputAndroid.cpp"
            "private/platforms/main_loop/MainLoop_Android.h" "private/platforms/main_loop/MainLoop_Android.cpp"
            "private/platforms/window/WinAndroid.h" "private/platforms/window/WinAndroid.cpp"
        )
    endif()

    if(APPLE)
        target_sources(${TARGET_NAME} PRIVATE
            # Core
            "private/core/io/apple/Path_Apple.mm"
        )

        if(IOS)
            target_sources(${TARGET_NAME} PRIVATE
                "private/platforms/config/ConfigiOS.h" "private/platforms/config/ConfigiOS.mm"
                "private/platforms/input/InputiOS.h" "private/platforms/input/InputiOS.mm"
                "private/platforms/Platform_iOS.mm"
                "private/platforms/main_loop/MainLoop_iOS.h" "private/platforms/main_loop/MainLoop_iOS.mm"
                "private/platforms/window/WiniOS.h" "private/platforms/window/WiniOS.mm"
            )
        else()
            target_sources(${TARGET_NAME} PRIVATE
                "private/platforms/config/ConfigMacOS.h" "private/platforms/config/ConfigMacOS.mm"
                "private/platforms/input/InputMacOS.h" "private/platforms/input/InputMacOS.mm"
                "private/platforms/Platform_MacOS.mm"
                "private/platforms/main_loop/MainLoop_MacOS.h" "private/platforms/main_loop/MainLoop_MacOS.mm"
                "private/platforms/window/WinMacOS.h" "private/platforms/window/WinMacOS.mm"
            )
        endif()
    endif()

    target_compile_features(${TARGET_NAME} PUBLIC cxx_std_23)

    if(UNIX AND NOT APPLE AND NOT ANDROID)
        target_link_libraries(${TARGET_NAME} PRIVATE wayland-client)

        set_source_files_properties(
            "private/core/specific/linux_wayland/xdg-shell-protocol.cpp"
            PROPERTIES COMPILE_OPTIONS "-Wno-attributes"
        )
    endif()

    if(ANDROID)
        find_package(game-activity REQUIRED CONFIG)
    endif()

    if(${IS_EDITOR})
        # common_lib_editor/logger_lib_editor - editor-варианты (см. src/common/CMakeLists.txt) с
        # EDITORDLL_EXPORTS, чтобы Z_COMMON_API (например throw_runtime_error) собрался как
        # dllexport и был виден из scripts.dll. Смешивать с обычными common_lib/logger_lib нельзя -
        # в editor_dll.dll попадут два набора одинаковых символов и линковщик упадёт на дублях.
        target_link_libraries(${TARGET_NAME}
            PUBLIC
                common_lib_editor
                logger_lib_editor
        )
    else()
        target_link_libraries(${TARGET_NAME}
            PUBLIC
                common_lib
                logger_lib
        )
    endif()

    target_compile_definitions(${TARGET_NAME} PRIVATE Z_TARGET_NAME="${TARGET_NAME}")
    set_source_files_properties(pch/pch.cpp PROPERTIES SKIP_PRECOMPILE_HEADERS ON)
    if(${IS_EDITOR})
        # EDITORDLL_EXPORTS должен совпадать с тем же define в editor_dll.vcxproj/CMakeLists.txt -
        # engine_lib_editor статический, но его символы (Script/ScriptRegistry, помеченные
        # Z_ENGINE_API) линкуются только в editor_dll.dll, иначе Z_ENGINE_API соберёт здесь
        # dllimport вместо dllexport (см. src/engine/public/core/userscripts/EngineExport.h).
        target_compile_definitions(${TARGET_NAME} PUBLIC Z_EDITOR=1 EDITORDLL_EXPORTS)
    endif()
endmacro()

setup_engine_target(engine_lib FALSE)
setup_engine_target(engine_lib_editor TRUE)

export_to_editor_sdk(engine_lib "${CMAKE_CURRENT_SOURCE_DIR}" "engine")
export_to_editor_sdk(engine_lib_editor "${CMAKE_CURRENT_SOURCE_DIR}" "engine")
