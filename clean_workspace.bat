@echo off
echo Cleaning temporary build directories and files...

REM Folders to delete
set FOLDERS=build .build .cache .docs bin out tmp .vs dist Testing CMakeFiles .idea
set FOLDERS=%FOLDERS% src\tools\bin src\tools\build
set FOLDERS=%FOLDERS% src\tools\build_configurator\bin src\tools\build_configurator\obj
set FOLDERS=%FOLDERS% src\tools\RemoteLogViewer\bin src\tools\RemoteLogViewer\obj
set FOLDERS=%FOLDERS% src\tools\editor\bin src\tools\editor\obj
set FOLDERS=%FOLDERS% src\tools\assets_builder\assets_builder_gui\bin src\tools\assets_builder\assets_builder_gui\obj
set FOLDERS=%FOLDERS% src\server\admin_panel_win\bin src\server\admin_panel_win\obj
set FOLDERS=%FOLDERS% src\projects\game_android\build src\projects\game_android\app\build
set FOLDERS=%FOLDERS% src\projects\game_android\app\.cxx src\projects\game_android\.idea
set FOLDERS=%FOLDERS% src\projects\game_android\.gradle
set FOLDERS=%FOLDERS% src\editor_projects\super_game\.editor\build src\editor_projects\super_game\.editor\bin

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
