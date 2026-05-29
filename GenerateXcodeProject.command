#!/bin/bash

cd "$(dirname "$0")"

echo "Generating macOS Xcode project..."
rm -rf projects/game_macos

cmake -B projects/game_macos -G Xcode

echo ""
echo "Generating iOS Xcode project..."

rm -rf projects/game_ios

cmake \
    -B projects/game_ios \
    -G Xcode \
    -DCMAKE_SYSTEM_NAME=iOS \
    -DCMAKE_OSX_DEPLOYMENT_TARGET=17.0

echo ""
echo "Done."