#!/bin/sh
#
# This script is used to compile your program on CodeCrafters
#
# This runs before .codecrafters/run.sh
#
# Learn more: https://codecrafters.io/program-interface

set -e # Exit on failure

# Only apply vcpkg toolchain if the environment variable and file exist
if [ -n "${VCPKG_ROOT}" ] && [ -f "${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" ]; then
  cmake -B build -S . \
        -DCMAKE_TOOLCHAIN_FILE=${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake
else
  cmake -B build -S .
fi

# Build the project
cmake --build ./build