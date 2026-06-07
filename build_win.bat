@echo off
call "C:\Program Files\Microsoft Visual Studio\18\Professional\VC\Auxiliary\Build\vcvars64.bat" > nul 2>&1
cd /d C:\Workspaces\ZzzTest
cmake --build build/Windows/debug
