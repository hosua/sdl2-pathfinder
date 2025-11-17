# Emscripten Build Instructions

This project can be compiled to WebAssembly using Emscripten, allowing it to run in a web browser.

## Prerequisites

1. **Install Emscripten SDK**
   - Visit: https://emscripten.org/docs/getting_started/downloads.html
   - Follow the installation instructions for your platform
   - Make sure `emcc` and `em++` are in your PATH

2. **Activate Emscripten environment**
   ```bash
   source /path/to/emsdk/emsdk_env.sh
   ```

## Building for Web

1. **Run the build script:**
   ```bash
   ./build_emscripten.sh
   ```

   This will:
   - Create a `build_emscripten` directory
   - Copy necessary files (assets, shell.html)
   - Configure and build the project with Emscripten
   - Generate `grid-game.js`, `grid-game.wasm`, and `grid-game.html`

2. **Test locally:**
   ```bash
   cd build_emscripten
   python3 -m http.server 8000
   ```
   Then open http://localhost:8000/grid-game.html in your browser

## Manual Build (Alternative)

If you prefer to build manually:

```bash
mkdir build_emscripten
cd build_emscripten
cp ../CMakeLists.emscripten.txt CMakeLists.txt
cp ../shell.html .
cp -r ../assets .

emcmake cmake ..
emmake make -j$(nproc)
```

## Notes

- The build uses `-sASYNCIFY` to handle async operations
- Assets are preloaded into the virtual file system at `/assets`
- The main loop uses `emscripten_set_main_loop` for browser compatibility
- Font paths are automatically adjusted for Emscripten's file system

## Troubleshooting

- **"emcc not found"**: Make sure Emscripten SDK is installed and activated
- **Font loading errors**: Ensure the `assets` directory is copied to the build directory
- **CORS errors**: Always serve files through an HTTP server, not `file://` protocol

