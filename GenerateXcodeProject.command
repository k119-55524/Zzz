#!/bin/bash

cd "$(dirname "$0")"

echo "Cleaning macOS CMake files..."
rm -rf projects/game_macos/CMakeFiles projects/game_macos/CMakeCache.txt projects/game_macos/*.xcodeproj projects/game_macos/build

echo "Generating macOS Xcode project..."
cmake -B projects/game_macos -G Xcode

echo ""
echo "Cleaning iOS CMake files..."
rm -rf projects/game_ios/CMakeFiles projects/game_ios/CMakeCache.txt projects/game_ios/*.xcodeproj projects/game_ios/build

echo "Generating iOS Xcode project..."
cmake \
    -B projects/game_ios \
    -G Xcode \
    -DCMAKE_SYSTEM_NAME=iOS \
    -DCMAKE_OSX_DEPLOYMENT_TARGET=17.0

echo ""
echo "Done."