# .NET SDK channel matching <TargetFramework> in RemoteLogViewer.csproj.
if(NOT DEFINED Z_DOTNET_CHANNEL)
    set(Z_DOTNET_CHANNEL "10.0")
endif()

find_program(DOTNET_EXECUTABLE dotnet)

if(NOT DOTNET_EXECUTABLE)
    # No system-wide SDK — bootstrap a private copy under build/ (already gitignored).
    # Re-running this on every configure also keeps that private copy updated to the
    # latest SDK patch in Z_DOTNET_CHANNEL, since the install script is idempotent
    # (it only downloads when the resolved version differs from what's installed).
    set(Z_DOTNET_LOCAL_DIR "${CMAKE_SOURCE_DIR}/build/.dotnet-local")

    if(WIN32)
        set(Z_DOTNET_LOCAL_EXE "${Z_DOTNET_LOCAL_DIR}/dotnet.exe")
        set(Z_DOTNET_INSTALL_SCRIPT "${CMAKE_SOURCE_DIR}/build/dotnet-install.ps1")
        file(DOWNLOAD "https://dot.net/v1/dotnet-install.ps1" "${Z_DOTNET_INSTALL_SCRIPT}" STATUS Z_DOTNET_DOWNLOAD_STATUS)
        list(GET Z_DOTNET_DOWNLOAD_STATUS 0 Z_DOTNET_DOWNLOAD_CODE)
        if(Z_DOTNET_DOWNLOAD_CODE EQUAL 0)
            execute_process(
                COMMAND powershell -NoProfile -ExecutionPolicy Bypass -File "${Z_DOTNET_INSTALL_SCRIPT}"
                        -Channel ${Z_DOTNET_CHANNEL} -InstallDir "${Z_DOTNET_LOCAL_DIR}"
                RESULT_VARIABLE Z_DOTNET_INSTALL_RESULT
            )
        endif()
    else()
        set(Z_DOTNET_LOCAL_EXE "${Z_DOTNET_LOCAL_DIR}/dotnet")
        set(Z_DOTNET_INSTALL_SCRIPT "${CMAKE_SOURCE_DIR}/build/dotnet-install.sh")
        file(DOWNLOAD "https://dot.net/v1/dotnet-install.sh" "${Z_DOTNET_INSTALL_SCRIPT}" STATUS Z_DOTNET_DOWNLOAD_STATUS)
        list(GET Z_DOTNET_DOWNLOAD_STATUS 0 Z_DOTNET_DOWNLOAD_CODE)
        if(Z_DOTNET_DOWNLOAD_CODE EQUAL 0)
            execute_process(COMMAND chmod +x "${Z_DOTNET_INSTALL_SCRIPT}")
            execute_process(
                COMMAND bash "${Z_DOTNET_INSTALL_SCRIPT}" --channel ${Z_DOTNET_CHANNEL} --install-dir "${Z_DOTNET_LOCAL_DIR}"
                RESULT_VARIABLE Z_DOTNET_INSTALL_RESULT
            )
        endif()
    endif()

    if(Z_DOTNET_DOWNLOAD_CODE EQUAL 0 AND Z_DOTNET_INSTALL_RESULT EQUAL 0 AND EXISTS "${Z_DOTNET_LOCAL_EXE}")
        set(DOTNET_EXECUTABLE "${Z_DOTNET_LOCAL_EXE}")
    else()
        message(WARNING "Не удалось автоматически установить .NET SDK (нет интернета или сбой dotnet-install) — цель RemoteLogViewer недоступна")
    endif()
endif()

if(DOTNET_EXECUTABLE)
    # Map CMake configuration to dotnet configuration (Debug or Release)
    set(DOTNET_CONFIG "$<IF:$<OR:$<CONFIG:Release>,$<CONFIG:MinSizeRel>,$<CONFIG:RelWithDebInfo>>,Release,Debug>")
    
    if(WIN32)
        set(SHORTCUT_PS1 "${CMAKE_CURRENT_BINARY_DIR}/CreateShortcut.ps1")
        file(WRITE "${SHORTCUT_PS1}"
"param([string]$Config)
$wshell = New-Object -ComObject WScript.Shell
$lnk = $wshell.CreateShortcut('${CMAKE_SOURCE_DIR}/RemoteLogViewer.lnk'.Replace('/', '\\'))
$lnk.TargetPath = '${CMAKE_SOURCE_DIR}/bin/' + $Config + '/RemoteLogViewer.exe'.Replace('/', '\\')
$lnk.WorkingDirectory = '${CMAKE_SOURCE_DIR}/bin/' + $Config.Replace('/', '\\')
$lnk.Save()
")
        add_custom_target(RemoteLogViewer ALL
            COMMAND ${DOTNET_EXECUTABLE} build -c ${DOTNET_CONFIG} "${CMAKE_CURRENT_SOURCE_DIR}/RemoteLogViewer.csproj"
            COMMAND powershell -NoProfile -ExecutionPolicy Bypass -File "${SHORTCUT_PS1}" -Config ${DOTNET_CONFIG}
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            COMMENT "Building RemoteLogViewer (C#/WPF) and creating shortcut"
            VERBATIM
        )
    else()
        add_custom_target(RemoteLogViewer ALL
            COMMAND ${DOTNET_EXECUTABLE} build -c ${DOTNET_CONFIG} "${CMAKE_CURRENT_SOURCE_DIR}/RemoteLogViewer.csproj"
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            COMMENT "Building RemoteLogViewer (C#/WPF)"
            VERBATIM
        )
    endif()
endif()
