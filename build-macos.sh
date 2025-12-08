#!/bin/bash

set -e

echo "Building Puzzles for macOS..."

# Create build directory
mkdir -p build/Release
cd build/Release

# Configure with CMake
cmake -S ../.. -B . -DCMAKE_BUILD_TYPE=Release -DCMAKE_OSX_DEPLOYMENT_TARGET=10.13

# Build
cmake --build . --parallel $(sysctl -n hw.ncpu)

cd ../..

# Package with Qt libraries
echo "Packaging app bundle with Qt libraries..."

APP_BUNDLE="build/Release/Puzzles.app"

# Use macdeployqt to bundle Qt frameworks
if command -v macdeployqt &> /dev/null; then
    macdeployqt "$APP_BUNDLE" -dmg
    echo "✓ Created Puzzles.dmg"
else
    echo "⚠ macdeployqt not found. Install Qt for macOS to create DMG."
    echo "App bundle available at: $APP_BUNDLE"
fi

echo "✓ Build complete!"
ls -lh "$APP_BUNDLE" 2>/dev/null || echo "App bundle created at $APP_BUNDLE"
