#!/bin/bash

set -e

echo "Cross-compiling Puzzles for Windows (x86_64)..."

# Verify MinGW is installed
if ! command -v x86_64-w64-mingw32-g++ &> /dev/null; then
    echo "❌ MinGW not found. Install with: sudo apt install mingw-w64"
    exit 1
fi

# Create build directory
mkdir -p build/Windows
cd build/Windows

# Download prebuilt Qt for Windows (if needed - this is a complex step)
# For now, we'll build with dynamic linking and provide instructions

echo "Configuring for Windows cross-compilation..."

# CMake toolchain for MinGW
cat > mingw-toolchain.cmake << 'EOF'
set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR x86_64)

set(CMAKE_C_COMPILER x86_64-w64-mingw32-gcc)
set(CMAKE_CXX_COMPILER x86_64-w64-mingw32-g++)
set(CMAKE_RC_COMPILER x86_64-w64-mingw32-windres)
set(CMAKE_FIND_ROOT_PATH /usr/x86_64-w64-mingw32)

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

# Qt6 settings for Windows
set(Qt6_DIR "/opt/qt-windows/lib/cmake/Qt6" CACHE PATH "Qt6 CMake path")
EOF

# Configure CMake with MinGW toolchain
cmake -S ../.. -B . \
    -DCMAKE_TOOLCHAIN_FILE=./mingw-toolchain.cmake \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_FIND_ROOT_PATH=/usr/x86_64-w64-mingw32

# Build
echo "Building..."
cmake --build . --parallel $(nproc) 2>&1 || {
    echo ""
    echo "⚠️  Build failed. Qt for Windows not found."
    echo ""
    echo "To complete the Windows build, you need to:"
    echo ""
    echo "Option 1: Install prebuilt Qt for Windows via MXE:"
    echo "  - Visit: https://mxe.cc/"
    echo "  - Or: sudo apt install mxe-x86-64-w64-mingw32-qt"
    echo ""
    echo "Option 2: Build on Windows directly:"
    echo "  - Use Visual Studio or MinGW on Windows"
    echo "  - Run: build-windows.bat"
    echo ""
    echo "Option 3: Use GitHub Actions (recommended):"
    echo "  - Push to GitHub with a git tag (v1.0.0)"
    echo "  - GitHub will automatically build Windows/macOS releases"
    exit 1
}

cd ../..

echo ""
echo "✓ Windows executable created!"
echo "  Location: build/Windows/Puzzles.exe"
echo ""
echo "Next step: Deploy Qt DLLs"
echo "  Download from: https://download.qt.io/official_releases/qt/6.7/6.7.0/single/"
echo "  Place DLLs in same directory as Puzzles.exe"
