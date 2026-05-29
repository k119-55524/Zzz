#!/bin/bash

cd "$(dirname "$0")"

echo "Generating macOS Xcode project..."

cmake -S . -B build/game_macos -G Xcode

echo ""
echo "Generating iOS Xcode project..."

cmake \
    -S . \
    -B build/game_ios \
    -G Xcode \
    -DCMAKE_SYSTEM_NAME=iOS \
    -DCMAKE_OSX_DEPLOYMENT_TARGET=17.0

echo ""
echo "Done."