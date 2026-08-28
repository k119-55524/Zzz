@echo off
setlocal

set SCRIPT_DIR=%~dp0
set ROOT_DIR=%SCRIPT_DIR%..

dotnet build "%ROOT_DIR%\src\tools\build_configurator\BuildConfigurator.csproj" -c Release
if %ERRORLEVEL% neq 0 (
    echo Build failed.
    pause
    exit /b %ERRORLEVEL%
)

echo.
echo Done. BuildConfigurator.exe is in the build_configs folder.
pause
