#pragma once

#include <optional>
#include <string>
#include <utility>
#include <vector>

struct CrosswordEntry {
    int number = 0;
    std::string word;
};

struct CrosswordPuzzle {
    std::vector<std::string> grid;
    std::vector<std::vector<int>> numbers;
    std::vector<CrosswordEntry> across;
    std::vector<CrosswordEntry> down;
};

std::optional<CrosswordPuzzle> generateCrossword(const std::vector<std::string>& rawWords, int rows, int cols);
