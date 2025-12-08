@echo off
REM Windows build script for Puzzles

setlocal enabledelayedexpansion

echo Building Puzzles for Windows...

REM Create build directory
if not exist "build\Release" mkdir build\Release
cd build\Release

REM Configure with CMake (Visual Studio 2022)
cmake -S ..\.. -B . -G "Visual Studio 17 2022" -DCMAKE_BUILD_TYPE=Release

REM Build
cmake --build . --config Release --parallel 4

REM Package with Qt (requires Qt deployed)
echo.
echo Packaging executable with Qt libraries...
REM Uncomment if you have Qt installed:
REM windeployqt.exe --release .\Release\Puzzles.exe

echo.
echo Build complete! Executable: .\Release\Puzzles.exe
pause
