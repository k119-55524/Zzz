call "C:\Program Files\Microsoft Visual Studio\18\Professional\VC\Auxiliary\Build\vcvars64.bat"
cmake -B .build/Windows/debug -S . -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build .build/Windows/debug
