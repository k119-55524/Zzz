@echo off
echo Cleaning temporary build directories and files...

REM Folders to delete
set "FOLDERS=.build .cache .docs bin out tmp .vs dist src\tools\build_configurator\bin src\tools\build_configurator\obj src\tools\RemoteLogViewer\bin src\tools\RemoteLogViewer\obj src\tools\editor\bin src\tools\editor\obj src\server\admin_panel_win\bin src\server\admin_panel_win\obj src\editor_projects\super_game\.editor\build src\editor_projects\super_game\.editor\bin CMakeFiles .idea"

for %%D in (%FOLDERS%) do (
    if exist "%%D" (
        echo Deleting directory: %%D
        rmdir /S /Q "%%D"
    )
)

REM Files to delete
set "FILES=CMakeCache.txt cmake_install.cmake compile_commands.json install_manifest.txt Makefile .ninja_deps .ninja_log build.ninja RemoteLogViewer.lnk BuildConfigurator.lnk BuildConfiguratorSwitch.lnk Documentation.lnk"

for %%F in (%FILES%) do (
    if exist "%%F" (
        echo Deleting file: %%F
        del /Q /F "%%F"
    )
)

REM Also clean any wpf tmp files and MSBuild logs
del /S /Q /F *_wpftmp.csproj 2>nul
del /Q /F *.log 2>nul

echo Clean complete!
pause
