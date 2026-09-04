add_subdirectory(server_lib)

if(WIN32)
    add_subdirectory(server_win)

    # Авто-сборка C# AdminPanelWin в CMake
    set(DOTNET_CONFIG "$<IF:$<OR:$<CONFIG:Release>,$<CONFIG:MinSizeRel>,$<CONFIG:RelWithDebInfo>>,Release,Debug>")
    add_custom_target(admin_panel_win_build ALL
        COMMAND dotnet build "${CMAKE_CURRENT_SOURCE_DIR}/admin_panel_win/AdminPanelWin.csproj" -c ${DOTNET_CONFIG}
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        COMMENT "Building AdminPanelWin C# WPF GUI..."
        VERBATIM
    )
endif()
