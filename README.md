Simple maze utility. Generates mazes with multiple algorithms, now shipped as a Qt desktop app. A legacy terminal build is available behind a flag.

## Building

```bash
cmake -S . -B build/Debug -DCMAKE_PREFIX_PATH=/path/to/Qt/lib/cmake
cmake --build build/Debug
```

Run the app:

`./build/Debug/MazeUtil`

Pick an algorithm, set the maze dimensions, click Generate, and move with WASD/arrow keys (or the on-screen buttons).

### Optional: terminal build

If you still want the terminal version, configure with `-DBUILD_CLI=ON` and run `./build/Debug/MazeUtilCli`.
