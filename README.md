Simple puzzle application. Generates a variety of simple paper puzzles. Ships as a Qt desktop app.

## Building

```bash
cmake -S . -B build/Debug -DCMAKE_PREFIX_PATH=/path/to/Qt/lib/cmake
cmake --build build/Debug
```

Run the app:

`./build/Debug/Puzzles`

Pick a puzzle type (maze, crossword, or word search), configure settings, click Create Puzzle, and interact with it.

## License

GPL-3.0-or-later. This aligns with Qt’s GPL offering; if you build against Qt under its GPL terms, this app remains compatible.
