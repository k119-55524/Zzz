macro(setup_engine_target TARGET_NAME IS_EDITOR)
    add_library(${TARGET_NAME})

    target_precompile_headers(${TARGET_NAME} PRIVATE
        pch/pch.h
    )

    target_include_directories(${TARGET_NAME}
        PUBLIC
            ${CMAKE_CURRENT_SOURCE_DIR}
            ${CMAKE_CURRENT_SOURCE_DIR}/scene
            ${CMAKE_CURRENT_SOURCE_DIR}/../core/userscripts
            ${CMAKE_CURRENT_SOURCE_DIR}/../core/userscripts/base_script
        PRIVATE
            pch
            ${CMAKE_CURRENT_SOURCE_DIR}/view
            ${CMAKE_CURRENT_SOURCE_DIR}/platforms
    )

    target_sources(${TARGET_NAME} PRIVATE
        # Public
        engine.h
        engine.cpp
        header.h
        NativeAppData.h
        pch/pch.h
        pch/pch.cpp

        # Core
        "platforms/package/PackageManager.h" "platforms/package/PackageManager.cpp"

        # Inputs
        "platforms/input/Input.h" "platforms/input/InputBase.h"

        # Platforms
        "platforms/Platform.h"
        "platforms/Platform_Common.cpp"

        # Main Loop
        "platforms/main_loop/MainLoop.h" "platforms/main_loop/MainLoop_Common.h" "platforms/main_loop/MainLoop_Common.cpp"
        "platforms/main_loop/MainLoop_Editor.h"

        # Native View / Window
        "platforms/window/Window.h" "platforms/window/Window_Common.h" "platforms/window/Window_Common.cpp"
        "view/View.h" "view/View.cpp"
        "view/ViewManager.h" "view/ViewManager.cpp"
        "../core/utils/ScreenResolution.h"

        # Wayland (Linux)
        "../core/specific/linux_wayland/xdg-shell-client-protocol.h"
        "../core/specific/linux_wayland/xdg-shell-protocol.cpp"
        "../core/utils/converters.h" "platforms/input/InputHelpers.h" "platforms/input/InputBase.cpp"

        # Scripts & Scenes
        "scene/GameObject.h"
        "scene/Scene.h" "scene/Scene.cpp"
        "../core/userscripts/ScriptRegistry.h" "../core/userscripts/ScriptRegistry.cpp"
        "../core/userscripts/base_script/Script.h"
        "../core/userscripts/base_script/Script.cpp"
        "../core/userscripts/base_script/GameScript.h"
        "../core/userscripts/base_script/GameScript.cpp"
        "../core/userscripts/base_script/SceneScript.h"
        "../core/userscripts/base_script/SceneScript.cpp"
        "../core/userscripts/base_script/ViewScript.h"
        "../core/userscripts/base_script/ViewScript.cpp")

    if(WIN32)
        if(${IS_EDITOR})
            target_sources(${TARGET_NAME} PRIVATE 
                "platforms/Platform_Editor.cpp"
                "platforms/input/InputEditor.h" "platforms/input/InputEditor.cpp"
                "platforms/window/WinMSWindowEditor.h" "platforms/window/WinMSWindowEditor.cpp"
            )
        else()
            target_sources(${TARGET_NAME} PRIVATE 
                "platforms/Platform_Windows.cpp"
                "platforms/input/InputMSWindows.h" "platforms/input/InputMSWindows.cpp"
                "platforms/main_loop/MainLoop_MSWin.h" "platforms/main_loop/MainLoop_MSWin.cpp"
                "platforms/window/WinMSWindows.h" "platforms/window/WinMSWindows.cpp"
            )
        endif()
    endif()

    if(UNIX AND NOT APPLE AND NOT ANDROID)
        target_sources(${TARGET_NAME} PRIVATE 
            "platforms/Platform_Linux.cpp"
            "platforms/input/InputLinux.h" "platforms/input/InputLinux.cpp"
            "platforms/main_loop/MainLoop_Linux.h" "platforms/main_loop/MainLoop_Linux.cpp"
            "platforms/window/WinLinux.h" "platforms/window/WinLinux.cpp"
        )
    endif()

    if(ANDROID)
        target_sources(${TARGET_NAME} PRIVATE 
            "platforms/Platform_Android.cpp"
            "platforms/input/InputAndroid.h" "platforms/input/InputAndroid.cpp"
            "platforms/main_loop/MainLoop_Android.h" "platforms/main_loop/MainLoop_Android.cpp"
            "platforms/window/WinAndroid.h" "platforms/window/WinAndroid.cpp"
        )
    endif()

    if(APPLE)
        if(IOS)
            target_sources(${TARGET_NAME} PRIVATE
                "platforms/input/InputiOS.h" "platforms/input/InputiOS.mm"
                "platforms/Platform_iOS.mm"
                "platforms/main_loop/MainLoop_iOS.h" "platforms/main_loop/MainLoop_iOS.mm"
                "platforms/window/WiniOS.h" "platforms/window/WiniOS.mm"
            )
        else()
            target_sources(${TARGET_NAME} PRIVATE
                "platforms/input/InputMacOS.h" "platforms/input/InputMacOS.mm"
                "platforms/Platform_MacOS.mm"
                "platforms/main_loop/MainLoop_MacOS.h" "platforms/main_loop/MainLoop_MacOS.mm"
                "platforms/window/WinMacOS.h" "platforms/window/WinMacOS.mm"
            )
        endif()
    endif()

    target_compile_features(${TARGET_NAME} PUBLIC cxx_std_23)

    if(UNIX AND NOT APPLE AND NOT ANDROID)
        target_link_libraries(${TARGET_NAME} PRIVATE wayland-client)

        set_source_files_properties(
            "../core/specific/linux_wayland/xdg-shell-protocol.cpp"
            PROPERTIES COMPILE_OPTIONS "-Wno-attributes"
        )
    endif()

    if(ANDROID)
        find_package(game-activity REQUIRED CONFIG)
    endif()

    if(${IS_EDITOR})
        # core_lib_editor/logger_lib_editor - editor-варианты с
        # EDITORDLL_EXPORTS, чтобы Z_CORE_API (например throw_runtime_error) собрался как
        # dllexport и был виден из scripts.dll. Смешивать с обычными logger_lib нельзя -
        # в editor_dll.dll попадут два набора одинаковых символов и линковщик упадёт на дублях.
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
    set_source_files_properties(pch/pch.cpp PROPERTIES SKIP_PRECOMPILE_HEADERS ON)
    if(${IS_EDITOR})
        # EDITORDLL_EXPORTS должен совпадать с тем же define в editor_dll.vcxproj/CMakeLists.txt -
        # engine_lib_editor статический, но его символы (Script/ScriptRegistry, помеченные
        # Z_ENGINE_API) линкуются только в editor_dll.dll, иначе Z_ENGINE_API соберёт здесь
        # dllimport вместо dllexport (см. src/core/userscripts/EngineExport.h).
        target_compile_definitions(${TARGET_NAME} PUBLIC Z_EDITOR=1 EDITORDLL_EXPORTS)
    endif()
endmacro()

setup_engine_target(engine_lib FALSE)
setup_engine_target(engine_lib_editor TRUE)

export_to_editor_sdk(engine_lib "${CMAKE_CURRENT_SOURCE_DIR}" "engine")
export_to_editor_sdk(engine_lib_editor "${CMAKE_CURRENT_SOURCE_DIR}" "engine")
