#!/bin/bash

cd "$(dirname "$0")"

echo "Cleaning macOS CMake files..."
rm -rf src/projects/game_macos/CMakeFiles src/projects/game_macos/CMakeCache.txt src/projects/game_macos/*.xcodeproj src/projects/game_macos/build

echo "Generating macOS Xcode project..."
cmake -B src/projects/game_macos -G Xcode

echo ""
echo "Cleaning iOS CMake files..."
rm -rf src/projects/game_ios/CMakeFiles src/projects/game_ios/CMakeCache.txt src/projects/game_ios/*.xcodeproj src/projects/game_ios/build

echo "Generating iOS Xcode project..."
cmake \
    -B src/projects/game_ios \
    -G Xcode \
    -DCMAKE_SYSTEM_NAME=iOS \
    -DCMAKE_OSX_DEPLOYMENT_TARGET=17.0

echo ""
echo "Done."