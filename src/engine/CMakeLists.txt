macro(setup_engine_target TARGET_NAME IS_EDITOR)
    add_library(${TARGET_NAME})

    target_precompile_headers(${TARGET_NAME} PRIVATE
        "pch/pch.h"
    )

    target_include_directories(${TARGET_NAME}
        PUBLIC
            ${CMAKE_CURRENT_SOURCE_DIR}/..
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}
            pch
            ${CMAKE_CURRENT_SOURCE_DIR}/scene
            ${CMAKE_CURRENT_SOURCE_DIR}/view
            ${CMAKE_CURRENT_SOURCE_DIR}/renderer
            ${CMAKE_CURRENT_SOURCE_DIR}/resources
            ${CMAKE_CURRENT_SOURCE_DIR}/platforms
    )

    target_sources(${TARGET_NAME} PRIVATE
        pch/pch.h
        pch/pch.cpp
        # Public
        engine.h
        engine.cpp
        EngineIncludes.h
        utils/EngineLogFlags.h
        utils/GpuUtils.h
        utils/MonitorUtils.h

        # Package & Settings
        "package/PackageManager.h" "package/PackageManager.cpp"
        "package/UserSettingsManager.h" "package/UserSettingsManager.cpp"

        # Resources (Этап 22)
        "resources/ResourceBase.h"
        "resources/ResourceRef.h"
        "resources/ResourceTable.h"
        "resources/cpu/CpuMesh.h" "resources/cpu/CpuMesh.cpp"
        "resources/cpu/CpuMaterial.h" "resources/cpu/CpuMaterial.cpp"
        "resources/cpu/CpuTexture2D.h" "resources/cpu/CpuTexture2D.cpp"
        "resources/cpu/CpuShader.h" "resources/cpu/CpuShader.cpp"
        "resources/cpu/loaders/CpuMeshLoader.h" "resources/cpu/loaders/CpuMeshLoader.cpp"
        "resources/cpu/loaders/CpuMaterialLoader.h" "resources/cpu/loaders/CpuMaterialLoader.cpp"
        "resources/cpu/loaders/CpuShaderLoader.h" "resources/cpu/loaders/CpuShaderLoader.cpp"
        "resources/cpu/CpuResourceManager.h" "resources/cpu/CpuResourceManager.cpp"
        "resources/gpu/GpuMesh.h" "resources/gpu/GpuMesh.cpp"
        "resources/gpu/GpuMaterial.h" "resources/gpu/GpuMaterial.cpp"
        "resources/gpu/GpuTexture2D.h" "resources/gpu/GpuTexture2D.cpp"
        "resources/gpu/GpuShader.h" "resources/gpu/GpuShader.cpp"
        "resources/gpu/GpuResourceManager.h" "resources/gpu/GpuResourceManager.cpp"

        # Inputs
        "platforms/input/Input.h" "platforms/input/InputBase.h"

        # Platforms
        "platforms/Platform.h"
        "platforms/PlatformCommon.cpp"
        "platforms/monitor/IMonitorProvider.h"
        "platforms/monitor/MonitorProviderMobile.h"
        "platforms/monitor/MonitorProviderLinux.h"
        "platforms/monitor/MonitorProviderMacOS.h"

        # Hardware (Пункт 6: HardwareManager)
        "platforms/hardware/HardwareManager.h" "platforms/hardware/HardwareManager.cpp"
        "platforms/hardware/CpuInfoCollector.h"
        "platforms/hardware/RamInfoCollector.h"
        "platforms/hardware/MotherboardInfoCollector.h"
        "platforms/hardware/GpuInfoCollector.h"
        "platforms/hardware/StorageInfoCollector.h"
        "platforms/hardware/NetworkAdapterInfoCollector.h"
        "platforms/hardware/platforms/motherboard/MotherboardInfoCollectorMobile.h"
        "platforms/hardware/platforms/storage/StorageInfoCollectorMobile.h"
        "platforms/hardware/platforms/network/NetworkAdapterInfoCollectorMobile.h"
        "platforms/hardware/platforms/gpu/GpuInfoCollectorStub.h"

        # Tasks (Этап 51)
        "tasks/TaskPriority.h"
        "tasks/TaskDispatcher.h" "tasks/TaskDispatcher.cpp"

        # Main Loop
        "platforms/mainloop/MainLoop.h" "platforms/mainloop/MainLoopCommon.h" "platforms/mainloop/MainLoopCommon.cpp"
        "platforms/mainloop/MainLoopEditor.h"

        # Native View / Window
        "platforms/window/NativeWindow.h" "platforms/window/NativeWindowState.h" "platforms/window/WindowCommon.h" "platforms/window/WindowCommon.cpp"
        "platforms/window/MacOSView.h"
        "platforms/window/iOSView.h"
        "view/View.h" "view/View.cpp"
        "view/ViewManager.h" "view/ViewManager.cpp"
        "view/ViewWindowState.h"

        # Helpers
        "platforms/input/InputHelpers.h" "platforms/input/InputBase.cpp"

        # Renderer
        "renderer/SceneRenderTree.h"
        "renderer/RenderManager.h" "renderer/RenderManager.cpp"

        # GAPI
        "gapi/IGAPI.h" "gapi/IGAPI.cpp"
        "gapi/ISurfView.h"
        "gapi/SurfView.h"
        "gapi/GAPI.h"
        "gapi/DepthBuffer.h"
        "gapi/Swapchain.h"
        "gapi/GAPIDebugLogger.h" "gapi/GAPIDebugLogger.cpp"
        "gapi/clear_config/ClearConfig.h"
        "gapi/clear_config/DepthBufferClearConfig.h"
        "gapi/clear_config/SurfaceClearConfig.h"
        "gapi/selectors/gpu/GpuRatingEvaluator.h"
        "gapi/selectors/gpu/IGpuSelector.h"
        "gapi/selectors/gpu/directx12/DirectX12GpuSelector.h" "gapi/selectors/gpu/directx12/DirectX12GpuSelector.cpp"
        "gapi/selectors/gpu/vulkan/VulkanGpuSelector.h" "gapi/selectors/gpu/vulkan/VulkanGpuSelector.cpp"
        "gapi/selectors/gpu/metal/MetalGpuSelector.h"
        "gapi/selectors/monitor/MonitorSelector.h"
        "gapi/vulkan/VulkanAPI.h" "gapi/vulkan/VulkanAPI.cpp"
        "gapi/vulkan/Swapchain_VK.h" "gapi/vulkan/Swapchain_VK.cpp"
        "gapi/vulkan/DepthBuffer_VK.h" "gapi/vulkan/DepthBuffer_VK.cpp"
        "gapi/vulkan/SurfView_VK.h" "gapi/vulkan/SurfView_VK.cpp"
        "gapi/directx12/DirectX12API.h" "gapi/directx12/DirectX12API.cpp"
        "gapi/directx12/Swapchain_DX.h" "gapi/directx12/Swapchain_DX.cpp"
        "gapi/directx12/DepthBuffer_DX.h" "gapi/directx12/DepthBuffer_DX.cpp"
        "gapi/directx12/SurfView_DX.h" "gapi/directx12/SurfView_DX.cpp"
        "gapi/metal/MetalAPI.h" "gapi/metal/MetalAPI.mm"
        "gapi/metal/SurfView_Metal.h" "gapi/metal/SurfView_Metal.mm"
        "gapi/metal/Swapchain_Metal.h"

        # Domain Interfaces & Implementations (Пункт 11.1 & 11.4)
        "scene/domain/ObjectDomain.h" "scene/domain/ObjectDomain.cpp"
        "scene/domain/IEntityDomain.h"
        "scene/domain/EntityDomain.h" "scene/domain/EntityDomain.cpp"
        "scene/domain/MVVMDomain.h"

        # Storage & Scene Tree (SoA-модель)
        "scene/storage/NodeTypes.h"
        "scene/storage/NodeStorage.h" "scene/storage/NodeStorage.cpp"

        # GameObject (Фасад над NodeStorage)
        "scene/gameobject/GameObject.h" "scene/gameobject/GameObject.cpp"

        # Entity (ECS-модель)
        "scene/entity/EntityWorld.h" "scene/entity/EntityWorld.cpp"

        # Storage (Пространственные хранилища)
        "scene/storage/ISpatialStorage.h"
        "scene/storage/DefaultSpatialStorage.h" "scene/storage/DefaultSpatialStorage.cpp"

        # Layers (Слои сцены, Пункт 11.4)
        "scene/layer/ILayer.h"
        "scene/layer/GameLayer.h" "scene/layer/GameLayer.cpp"
        "scene/layer/LayerMVVM.h" "scene/layer/LayerMVVM.cpp"
        "scene/layer/LayerSubsystemFactory.h" "scene/layer/LayerSubsystemFactory.cpp"
        "scene/Scene.h" "scene/Scene.cpp"
        "scene/SceneManager.h" "scene/SceneManager.cpp")


    if(WIN32)
        if(${IS_EDITOR})
            target_sources(${TARGET_NAME} PRIVATE 
                "platforms/PlatformEditor.cpp"
                "platforms/input/InputEditor.h" "platforms/input/InputEditor.cpp"
                "platforms/window/WinMSWindowEditor.h" "platforms/window/WinMSWindowEditor.cpp"
                "platforms/monitor/MonitorProviderMSWin.h" "platforms/monitor/MonitorProviderMSWin.cpp"
                "platforms/hardware/platforms/cpu/CpuInfoCollectorEditor.h"
                "platforms/hardware/platforms/ram/RamInfoCollectorEditor.h"
                "platforms/hardware/platforms/motherboard/MotherboardInfoCollectorEditor.h"
                "platforms/hardware/platforms/gpu/GpuInfoCollectorEditor.h"
                "platforms/hardware/platforms/storage/StorageInfoCollectorEditor.h"
                "platforms/hardware/platforms/network/NetworkAdapterInfoCollectorEditor.h"
            )
        else()
            target_sources(${TARGET_NAME} PRIVATE 
                "platforms/PlatformWindows.cpp"
                "platforms/input/InputMSWindows.h" "platforms/input/InputMSWindows.cpp"
                "platforms/mainloop/MainLoopMSWin.h" "platforms/mainloop/MainLoopMSWin.cpp"
                "platforms/window/WinMSWindows.h" "platforms/window/WinMSWindows.cpp"
                "platforms/monitor/MonitorProviderMSWin.h" "platforms/monitor/MonitorProviderMSWin.cpp"
                "platforms/hardware/platforms/common/WCharUtilsMSWin.h"
                "platforms/hardware/platforms/common/SmbiosReaderMSWin.h" "platforms/hardware/platforms/common/SmbiosReaderMSWin.cpp"
                "platforms/hardware/platforms/cpu/CpuInfoCollectorMSWin.h" "platforms/hardware/platforms/cpu/CpuInfoCollectorMSWin.cpp"
                "platforms/hardware/platforms/ram/RamInfoCollectorMSWin.h" "platforms/hardware/platforms/ram/RamInfoCollectorMSWin.cpp"
                "platforms/hardware/platforms/motherboard/MotherboardInfoCollectorMSWin.h" "platforms/hardware/platforms/motherboard/MotherboardInfoCollectorMSWin.cpp"
                "platforms/hardware/platforms/storage/StorageInfoCollectorMSWin.h" "platforms/hardware/platforms/storage/StorageInfoCollectorMSWin.cpp"
                "platforms/hardware/platforms/network/NetworkAdapterInfoCollectorMSWin.h" "platforms/hardware/platforms/network/NetworkAdapterInfoCollectorMSWin.cpp"
                "platforms/hardware/platforms/gpu/GpuInfoCollectorMSWin.h" "platforms/hardware/platforms/gpu/GpuInfoCollectorMSWin.cpp"
            )
        endif()
    endif()

    if(UNIX AND NOT APPLE AND NOT ANDROID)
        target_sources(${TARGET_NAME} PRIVATE 
            "platforms/PlatformLinux.cpp"
            "platforms/input/InputLinux.h" "platforms/input/InputLinux.cpp"
            "platforms/mainloop/MainLoopLinux.h" "platforms/mainloop/MainLoopLinux.cpp"
            "platforms/window/WinLinux.h" "platforms/window/WinLinux.cpp"
            "platforms/hardware/platforms/cpu/CpuInfoCollectorLinux.h" "platforms/hardware/platforms/cpu/CpuInfoCollectorLinux.cpp"
            "platforms/hardware/platforms/ram/RamInfoCollectorLinux.h" "platforms/hardware/platforms/ram/RamInfoCollectorLinux.cpp"
            "platforms/hardware/platforms/motherboard/MotherboardInfoCollectorLinux.h" "platforms/hardware/platforms/motherboard/MotherboardInfoCollectorLinux.cpp"
            "platforms/hardware/platforms/storage/StorageInfoCollectorLinux.h" "platforms/hardware/platforms/storage/StorageInfoCollectorLinux.cpp"
            "platforms/hardware/platforms/network/NetworkAdapterInfoCollectorLinux.h" "platforms/hardware/platforms/network/NetworkAdapterInfoCollectorLinux.cpp"
        )
    endif()

    if(ANDROID)
        target_sources(${TARGET_NAME} PRIVATE 
            "platforms/PlatformAndroid.cpp"
            "platforms/input/InputAndroid.h" "platforms/input/InputAndroid.cpp"
            "platforms/mainloop/MainLoopAndroid.h" "platforms/mainloop/MainLoopAndroid.cpp"
            "platforms/window/WinAndroid.h" "platforms/window/WinAndroid.cpp"
            "platforms/hardware/platforms/cpu/CpuInfoCollectorAndroid.h" "platforms/hardware/platforms/cpu/CpuInfoCollectorAndroid.cpp"
            "platforms/hardware/platforms/ram/RamInfoCollectorAndroid.h" "platforms/hardware/platforms/ram/RamInfoCollectorAndroid.cpp"
        )
    endif()

    if(APPLE)
        if(IOS)
            target_sources(${TARGET_NAME} PRIVATE
                "platforms/input/InputiOS.h" "platforms/input/InputiOS.mm"
                "platforms/PlatformiOS.mm"
                "platforms/mainloop/MainLoopiOS.h" "platforms/mainloop/MainLoopiOS.mm"
                "platforms/window/WiniOS.h" "platforms/window/WiniOS.mm"
                "platforms/hardware/platforms/cpu/CpuInfoCollectorApple.h" "platforms/hardware/platforms/cpu/CpuInfoCollectorApple.cpp"
                "platforms/hardware/platforms/ram/RamInfoCollectorApple.h" "platforms/hardware/platforms/ram/RamInfoCollectorApple.cpp"
            )
        else()
            target_sources(${TARGET_NAME} PRIVATE
                "platforms/input/InputMacOS.h" "platforms/input/InputMacOS.mm"
                "platforms/PlatformMacOS.mm"
                "platforms/mainloop/MainLoopMacOS.h" "platforms/mainloop/MainLoopMacOS.mm"
                "platforms/window/WinMacOS.h" "platforms/window/WinMacOS.mm"
                "platforms/hardware/platforms/cpu/CpuInfoCollectorApple.h" "platforms/hardware/platforms/cpu/CpuInfoCollectorApple.cpp"
                "platforms/hardware/platforms/ram/RamInfoCollectorApple.h" "platforms/hardware/platforms/ram/RamInfoCollectorApple.cpp"
                "platforms/hardware/platforms/motherboard/MotherboardInfoCollectorMacOS.h" "platforms/hardware/platforms/motherboard/MotherboardInfoCollectorMacOS.cpp"
                "platforms/hardware/platforms/storage/StorageInfoCollectorMacOS.h" "platforms/hardware/platforms/storage/StorageInfoCollectorMacOS.cpp"
                "platforms/hardware/platforms/network/NetworkAdapterInfoCollectorMacOS.h" "platforms/hardware/platforms/network/NetworkAdapterInfoCollectorMacOS.cpp"
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
    set_source_files_properties(pch/pch.cpp PROPERTIES SKIP_PRECOMPILE_HEADERS ON)
    if(${IS_EDITOR})
        target_compile_definitions(${TARGET_NAME} PUBLIC Z_EDITOR=1 EDITORDLL_EXPORTS)
    endif()
endmacro()

setup_engine_target(engine_lib FALSE)
setup_engine_target(engine_lib_editor TRUE)
