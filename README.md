Puzzles - A Qt-based puzzle generator and solver for creating and playing various puzzle types.

## What's Included

- **Mazes** - 6 different generation algorithms to choose from
- **Crosswords** - Auto-generated with hints
- **Word Search** - Customizable grids
- **Sudoku** - Three difficulty levels
- **Cryptograms** - Letter substitution puzzles

## Features

- Generate puzzles with customizable parameters
- Interactive test mode with keyboard controls (WASD or Arrow keys)
- Customize wall and background colors
- Zoom and pan controls
- Save and load puzzle state
- Export puzzles as images
- Built-in puzzle library with save management

## Building

```bash
cmake -S . -B build/Debug -DCMAKE_PREFIX_PATH=/path/to/Qt/lib/cmake
cmake --build build/Debug
```

Run:
```bash
./build/Debug/Puzzles
```

Pick a puzzle type, set your preferences, create it, and solve. Right-click saved puzzles in the list for test mode and export options.

## License

GPL-3.0
