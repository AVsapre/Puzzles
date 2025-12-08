#!/bin/bash

set -e

echo "Building Puzzles for Windows (cross-compile from Linux)..."
echo "⚠ Note: This requires MXE or MinGW toolchain installed"

# Check for MinGW
if ! command -v x86_64-w64-mingw32-gcc &> /dev/null; then
    echo "MinGW not found. Install with:"
    echo "  Ubuntu/Debian: sudo apt install mingw-w64"
    echo "  Fedora: sudo dnf install mingw64-gcc mingw64-gcc-c++"
    echo "  macOS: brew install mingw-w64"
    exit 1
fi

# Create build directory
mkdir -p build/Windows
cd build/Windows

# Configure with MinGW toolchain
cmake -S ../.. -B . \
    -DCMAKE_TOOLCHAIN_FILE=/usr/share/mingw/toolchain-x86_64-w64-mingw32.cmake \
    -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build . --parallel $(nproc)

cd ../..

echo "✓ Windows executable created: build/Windows/Puzzles.exe"
echo "⚠ Note: You'll need to manually deploy Qt DLLs with the executable"
