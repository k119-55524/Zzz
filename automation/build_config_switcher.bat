@echo off
setlocal

set SCRIPT_DIR=%~dp0
set ROOT_DIR=%SCRIPT_DIR%..

cmake -S "%ROOT_DIR%\src\tools\config_switcher" -B "%ROOT_DIR%\src\tools\config_switcher\build" -A x64
if %ERRORLEVEL% neq 0 (
    echo CMake configure failed.
    pause
    exit /b %ERRORLEVEL%
)

cmake --build "%ROOT_DIR%\src\tools\config_switcher\build" --config Release
if %ERRORLEVEL% neq 0 (
    echo CMake build failed.
    pause
    exit /b %ERRORLEVEL%
)

echo.
echo Done. build_configurator_switch.exe is in the build_configs folder.
pause
