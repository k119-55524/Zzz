@echo off

cmake -B build ^
	-G Ninja ^
	-DZZZ_ENABLE_LOGGER=OFF
	
cmake --build build

pause