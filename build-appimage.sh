#!/bin/bash

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
APPDIR="$SCRIPT_DIR/AppDir"

echo "Building Puzzles AppImage..."

# Clean AppDir
rm -rf "$APPDIR"
mkdir -p "$APPDIR"

# Build in Release mode for distribution
mkdir -p build/Release
cd build/Release

# Configure with Release build type
cmake -S ../.. -B . -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr

# Build
cmake --build . --parallel $(nproc)

# Install to AppDir
cmake --install . --prefix "$APPDIR/usr"

cd "$SCRIPT_DIR"

# Ensure proper directory structure
mkdir -p "$APPDIR/usr/share/applications"
mkdir -p "$APPDIR/usr/share/icons/hicolor/256x256/apps"
mkdir -p "$APPDIR/usr/lib"

# Copy desktop and icon files
cp puzzles.desktop "$APPDIR/puzzles.desktop"
cp puzzles.desktop "$APPDIR/usr/share/applications/puzzles.desktop"
cp icon/logo.png "$APPDIR/usr/share/pixmaps/puzzles.png"
cp icon/logo.png "$APPDIR/usr/share/icons/hicolor/256x256/apps/puzzles.png"
cp icon/logo.png "$APPDIR/puzzles.png"

# Create AppRun script
cat > "$APPDIR/AppRun" << 'EOF'
#!/bin/bash
HERE="$(cd "$(dirname "$0")" && pwd)"
export LD_LIBRARY_PATH="$HERE/usr/lib:$HERE/usr/lib/x86_64-linux-gnu:$LD_LIBRARY_PATH"
export QT_PLUGIN_PATH="$HERE/usr/lib/qt6/plugins"
exec "$HERE/usr/bin/Puzzles" "$@"
EOF
chmod +x "$APPDIR/AppRun"

# Copy Qt libraries
echo "Copying Qt libraries..."
QT_LIB_PATH=$(qmake -query QT_INSTALL_LIBS 2>/dev/null || echo "/usr/lib/x86_64-linux-gnu")

for lib in libQt6Core libQt6Gui libQt6Widgets libQt6PrintSupport; do
    if [ -f "$QT_LIB_PATH/$lib.so.6" ]; then
        cp "$QT_LIB_PATH/$lib.so.6" "$APPDIR/usr/lib/" 2>/dev/null || true
    fi
done

# Copy Qt plugins
echo "Copying Qt plugins..."
QT_PLUGINS=$(qmake -query QT_INSTALL_PLUGINS 2>/dev/null || echo "/usr/lib/x86_64-linux-gnu/qt6/plugins")

for plugin_dir in imageformats platforms; do
    if [ -d "$QT_PLUGINS/$plugin_dir" ]; then
        mkdir -p "$APPDIR/usr/lib/qt6/plugins/$plugin_dir"
        cp "$QT_PLUGINS/$plugin_dir"/*.so "$APPDIR/usr/lib/qt6/plugins/$plugin_dir/" 2>/dev/null || true
    fi
done

# Download appimagetool if not present
if [ ! -f appimagetool-x86_64.AppImage ]; then
    echo "Downloading appimagetool..."
    wget -q https://github.com/AppImage/AppImageKit/releases/download/continuous/appimagetool-x86_64.AppImage
    chmod +x appimagetool-x86_64.AppImage
fi

# Create AppImage
echo "Creating AppImage..."
./appimagetool-x86_64.AppImage "$APPDIR" Puzzles-1.0.0-x86_64.AppImage

echo "✓ AppImage created successfully!"
ls -lh Puzzles-*.AppImage


