#!/usr/bin/env bash
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$SCRIPT_DIR/.."

cmake -S "$ROOT_DIR/tools/config_switcher" -B "$ROOT_DIR/tools/config_switcher/build"
cmake --build "$ROOT_DIR/tools/config_switcher/build" --config Release

echo ""
echo "Done. build_configurator_switch is in the repository root."
