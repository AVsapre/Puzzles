#pragma once

#include <optional>
#include <vector>

struct SudokuPuzzle {
    std::vector<std::vector<int>> grid;
    std::vector<std::vector<int>> solution;
};

std::optional<SudokuPuzzle> generateSudoku(int difficulty);
