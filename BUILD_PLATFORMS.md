# Cross-Platform Build Instructions

## Linux (AppImage) ✅

```bash
./build-appimage.sh
```
Creates: `Puzzles-1.0.0-x86_64.AppImage` (9.4 MB)
- Works on any Linux with glibc 2.29+
- All dependencies bundled
- Ready to distribute

## Windows (EXE)

### Option 1: Build on Windows (Recommended)

**Requirements:**
- Windows 10/11
- Visual Studio 2022 Community (free)
- Qt 6 for Windows
- CMake 3.16+

**Steps:**
```cmd
build-windows.bat
```

Then deploy Qt libraries:
```cmd
windeployqt.exe --release build\Release\Puzzles.exe --dir build\Release
```

Creates: `build\Release\Puzzles.exe` with all dependencies

### Option 2: Cross-compile from Linux

Requires Qt built for Windows. Install MXE:
```bash
# Ubuntu/Debian
sudo apt install mxe-x86-64-w64-mingw32-qt

# Then cross-compile
./build-windows-linux.sh
```

⚠️ **Easier alternative:** Use GitHub Actions (see below)

### Option 3: Use GitHub Actions (Easiest!) ⭐

Push a git tag to GitHub and let CI build it:
```bash
git tag v1.0.0
git push origin v1.0.0
```

GitHub Actions will automatically:
- ✓ Build Windows EXE
- ✓ Build macOS DMG  
- ✓ Build Linux AppImage
- ✓ Create Release with all artifacts

## macOS (App Bundle / DMG)

### Option 1: Build on macOS (Recommended)

**Requirements:**
- macOS 10.13+
- Xcode Command Line Tools
- Qt 6 for macOS

**Install Qt:**
```bash
brew install qt
```

**Build:**
```bash
chmod +x build-macos.sh
./build-macos.sh
```

Creates: `Puzzles.dmg` for distribution

### Option 2: Use GitHub Actions

Same as Windows - push a tag:
```bash
git tag v1.0.0
git push origin v1.0.0
```

GitHub will build the `.dmg` automatically.

## Recommended: GitHub Actions (All Platforms) ⭐

The easiest way to build for all platforms:

1. **Commit and push your changes:**
```bash
git add .
git commit -m "Add cross-platform build support"
git push
```

2. **Create a release tag:**
```bash
git tag v1.0.0
git push origin v1.0.0
```

3. **GitHub Actions automatically builds:**
   - ✓ Linux AppImage (Ubuntu)
   - ✓ macOS DMG (macOS Latest)
   - ✓ Windows EXE (Windows Latest)
   - ✓ Creates GitHub Release with all 3 artifacts

4. **Download from:** https://github.com/AVsapre/Puzzles/releases/tag/v1.0.0

## Distribution Summary

| Platform | Format | Build Method | Status |
|----------|--------|--------------|--------|
| Linux | AppImage | `./build-appimage.sh` | ✅ Ready |
| Windows | EXE | `build-windows.bat` (on Windows) | ⚠️ Needs Windows |
| macOS | DMG | `./build-macos.sh` (on macOS) | ⚠️ Needs macOS |
| All 3 | All | GitHub Actions CI | ✅ Automated |

## Creating Windows Installer (Optional)

Once you have `Puzzles.exe`, you can create an installer using NSIS:

1. Install NSIS from https://nsis.sourceforge.io/
2. Create `installer.nsi`:

```nsis
!include "MUI2.nsh"

Name "Puzzles 1.0.0"
OutFile "Puzzles-1.0.0-Setup.exe"
InstallDir "$PROGRAMFILES\Puzzles"
!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_LANGUAGE "English"

Section "Install"
  SetOutPath "$INSTDIR"
  File "build\Release\Puzzles.exe"
  File "build\Release\*.dll"
  CreateShortCut "$SMPROGRAMS\Puzzles.lnk" "$INSTDIR\Puzzles.exe"
  CreateShortCut "$DESKTOP\Puzzles.lnk" "$INSTDIR\Puzzles.exe"
SectionEnd
```

3. Build installer:
```cmd
makensis installer.nsi
```

Creates: `Puzzles-1.0.0-Setup.exe`

## Best Practices

1. **Always build in Release mode** for distribution
2. **Test on target systems** before releasing
3. **Include README** with system requirements
4. **Sign executables** (optional but recommended):
   - Windows: DigiCert certificate
   - macOS: Apple Developer account
5. **Version your releases** with semantic versioning (v1.0.0)

