#!/bin/bash

# Build script for Emscripten

set -e

# Source shell configuration files to get PATH and aliases
if [ -f ~/.bashrc ]; then
    source ~/.bashrc 2>/dev/null || true
fi
if [ -f ~/.bash_profile ]; then
    source ~/.bash_profile 2>/dev/null || true
fi
if [ -f ~/.profile ]; then
    source ~/.profile 2>/dev/null || true
fi

# Try to find and add Emscripten to PATH if not already there
if ! command -v emcc &> /dev/null; then
    # Common Emscripten installation paths
    for emsdk_path in \
        "/usr/lib/emsdk/upstream/emscripten" \
        "$HOME/emsdk/upstream/emscripten" \
        "$HOME/.emsdk/upstream/emscripten" \
        "/opt/emsdk/upstream/emscripten"; do
        if [ -d "$emsdk_path" ] && [ -f "$emsdk_path/emcc" ]; then
            export PATH="$PATH:$emsdk_path"
            break
        fi
    done
    
    # Also try to source emsdk_env.sh if available
    for emsdk_env in \
        "$HOME/emsdk/emsdk_env.sh" \
        "$HOME/.emsdk/emsdk_env.sh" \
        "/opt/emsdk/emsdk_env.sh" \
        "/usr/lib/emsdk/emsdk_env.sh"; do
        if [ -f "$emsdk_env" ]; then
            source "$emsdk_env" > /dev/null 2>&1
            break
        fi
    done
fi

# Store original directory
ORIG_DIR="$(pwd)"

# Cleanup function to restore CMakeLists.txt
cleanup() {
    cd "$ORIG_DIR"
    if [ -f "CMakeLists.txt.backup" ]; then
        mv CMakeLists.txt.backup CMakeLists.txt
        echo "Restored original CMakeLists.txt"
    fi
}

# Set trap to ensure cleanup on exit
trap cleanup EXIT

# Check if emcc is available
if ! command -v emcc &> /dev/null; then
    echo "Error: emcc not found. Please install Emscripten SDK."
    echo "Visit: https://emscripten.org/docs/getting_started/downloads.html"
    exit 1
fi

# Create build directory
BUILD_DIR="build_emscripten"
mkdir -p "$BUILD_DIR"

# Copy shell.html to build directory (will be used by CMake)
cp shell.html "$BUILD_DIR/"

# Copy assets directory to build directory
if [ -d "assets" ]; then
    cp -r assets "$BUILD_DIR/"
else
    echo "Warning: assets directory not found"
fi

# Backup original CMakeLists.txt and use emscripten version
cd "$BUILD_DIR/.."
if [ -f "CMakeLists.txt" ] && [ ! -f "CMakeLists.txt.backup" ]; then
    cp CMakeLists.txt CMakeLists.txt.backup
fi
cp CMakeLists.emscripten.txt CMakeLists.txt

# Change to build directory
cd "$BUILD_DIR"

# Configure with Emscripten
emcmake cmake ..

# Build
emmake make -j$(nproc)

# The output will be grid-game.js, grid-game.wasm, and grid-game.html
echo ""
echo "Build complete! Output files:"
echo "  - grid-game.js"
echo "  - grid-game.wasm"
echo "  - grid-game.html"
echo ""
echo "To test locally, you can use a simple HTTP server:"
echo "  cd $BUILD_DIR && python3 -m http.server 8000"
echo "Then open http://localhost:8000/grid-game.html in your browser"

