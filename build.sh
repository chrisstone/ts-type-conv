#!/usr/bin/env bash
# ============================================================================
# @file    build.sh
# @author  Chris Stone
# @version 0.1.1
# @brief   Builds the ts-type-conv project on Linux, MacOS, and FreeBSD.
# ============================================================================

set -euo pipefail

TARGET_DIR="build"

mkdir -p "$TARGET_DIR"
cd "$TARGET_DIR"

cmake ..
cmake --build . --config Release
ctest -C Release --output-on-failure --output-junit test.xml -O test.log
