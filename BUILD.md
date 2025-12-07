# Building Puzzles on All Platforms

## Prerequisites

### Windows
```batch
# Using vcpkg (recommended)
vcpkg install qt6:x64-windows cmake
# or using Chocolatey
choco install qt-online cmake
```

### macOS
```bash
brew install qt@6 cmake
```

### Linux (Ubuntu/Debian)
```bash
sudo apt-get install qt6-base-dev qt6-tools-dev cmake build-essential
```

### Linux (Fedora/RHEL)
```bash
sudo dnf install qt6-qtbase-devel qt6-qttools-devel cmake gcc-c++
```

## Building

### All Platforms - GUI Build
```bash
cmake -S . -B build/Debug -DCMAKE_BUILD_TYPE=Debug
cmake --build build --config Debug
```

Run GUI:
- **Windows**: `build\Debug\Puzzles.exe`
- **macOS/Linux**: `./build/Debug/Puzzles`

### Release Build (All Platforms)
```bash
cmake -S . -B build/Release -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

Run Release:
- **Windows**: `build\Release\Puzzles.exe`
- **macOS/Linux**: `./build/Release/Puzzles`

## Troubleshooting

### Qt not found (All Platforms)
```bash
# Set Qt path explicitly
cmake -S . -B build/Debug -DCMAKE_PREFIX_PATH=/path/to/qt6
```

### On macOS: "Qt6 not found"
```bash
export PATH="/usr/local/opt/qt@6/bin:$PATH"
cmake -S . -B build/Debug -DCMAKE_BUILD_TYPE=Debug
```

### On Windows: MSVC Compiler Issues
```batch
# Use Visual Studio 16 or 17
cmake -S . -B build -G "Visual Studio 17 2022"
cmake --build build --config Debug
```

### On Linux: Missing dependencies
```bash
# Ubuntu/Debian
sudo apt-get update && sudo apt-get install -y qt6-base-dev qt6-tools-dev

# Fedora
sudo dnf groupinstall "Development Tools"
sudo dnf install qt6-qtbase-devel
```
