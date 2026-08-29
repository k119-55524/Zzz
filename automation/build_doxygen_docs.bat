@echo off
setlocal EnableDelayedExpansion

title Zzz Engine - Doxygen Documentation Generator

REM Switch working directory to repository root
cd /d "%~dp0\.."

echo =====================================================================
echo  Zzz Engine - Doxygen Documentation Generator
echo =====================================================================
echo.
echo [1/2] Generating API documentation with Doxygen ^& Graphviz...

REM Ensure MSVC compiler environment is available if cl.exe is not in PATH
where cl.exe >nul 2>&1
if %ERRORLEVEL% neq 0 (
    if exist "C:\Program Files\Microsoft Visual Studio\18\Professional\VC\Auxiliary\Build\vcvars64.bat" (
        call "C:\Program Files\Microsoft Visual Studio\18\Professional\VC\Auxiliary\Build\vcvars64.bat" >nul
    ) else if exist "C:\Program Files\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build\vcvars64.bat" (
        call "C:\Program Files\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build\vcvars64.bat" >nul
    ) else if exist "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat" (
        call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat" >nul
    )
)

set "BUILD_DIR=.build\Windows\debug"
if exist ".build\Windows\development\build.ninja" (
    set "BUILD_DIR=.build\Windows\development"
)

if not exist "!BUILD_DIR!\build.ninja" (
    echo Configuring CMake in !BUILD_DIR!...
    cmake -B !BUILD_DIR! -S . -G Ninja -DCMAKE_BUILD_TYPE=Debug
    if errorlevel 1 (
        echo [ERROR] CMake configuration failed!
        pause
        exit /b 1
    )
)

echo Building CMake target 'docs'...
cmake --build !BUILD_DIR! --target docs
if errorlevel 1 (
    echo [ERROR] Failed to generate Doxygen documentation!
    pause
    exit /b 1
)

echo [2/2] Creating root shortcut and opening documentation...
if exist ".docs\html\index.html" (
    powershell -NoProfile -Command "$ws = New-Object -ComObject WScript.Shell; $s = $ws.CreateShortcut((Join-Path (Get-Location) 'Documentation.lnk')); $s.TargetPath = (Join-Path (Get-Location) '.docs\html\index.html'); $s.Save()" >nul 2>&1
    start .docs\html\index.html
    echo.
    echo =====================================================================
    echo  Documentation ready!
    echo  Shortcut created: Documentation.lnk in project root
    echo =====================================================================
) else (
    echo [WARNING] .docs\html\index.html not found!
)

endlocal
