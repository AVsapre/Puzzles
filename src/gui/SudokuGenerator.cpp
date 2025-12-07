#include "SudokuGenerator.h"

#include <vector>
#include <algorithm>
#include <random>


bool solveSudoku(std::vector<std::vector<int>>& board) {
    for (int row = 0; row < 9; ++row) {
        for (int col = 0; col < 9; ++col) {
            if (board[row][col] == 0) {
                std::vector<int> numbers = {1, 2, 3, 4, 5, 6, 7, 8, 9};
                std::random_device rd;
                std::mt19937 g(rd());
                std::shuffle(numbers.begin(), numbers.end(), g);
                for (int num : numbers) {
                    bool possible = true;
                    for (int i = 0; i < 9; ++i) {
                        if (board[row][i] == num || board[i][col] == num) {
                            possible = false;
                            break;
                        }
                    }
                    if (possible) {
                        int box_row = row / 3;
                        int box_col = col / 3;
                        for (int i = 0; i < 3; ++i) {
                            for (int j = 0; j < 3; ++j) {
                                if (board[box_row * 3 + i][box_col * 3 + j] == num) {
                                    possible = false;
                                    break;
                                }
                            }
                            if (!possible)
                                break;
                        }
                    }

                    if (possible) {
                        board[row][col] = num;
                        if (solveSudoku(board)) {
                            return true;
                        }
                        board[row][col] = 0;
                    }
                }
                return false;
            }
        }
    }
    return true;
}

std::optional<SudokuPuzzle> generateSudoku(int difficulty) {
    SudokuPuzzle puzzle;
    puzzle.grid.assign(9, std::vector<int>(9, 0));
    puzzle.solution.assign(9, std::vector<int>(9, 0));

    solveSudoku(puzzle.solution);
    puzzle.grid = puzzle.solution;

    int cells_to_remove = 0;
    if (difficulty == 0) { 
        cells_to_remove = 40;
    } else if (difficulty == 1) { 
        cells_to_remove = 50;
    } else { 
        cells_to_remove = 60;
    }

    std::random_device rd;
    std::mt19937 g(rd());
    std::vector<int> indices(81);
    std::iota(indices.begin(), indices.end(), 0);
    std::shuffle(indices.begin(), indices.end(), g);
    
    int removed_count = 0;
    for (int i = 0; i < 81 && removed_count < cells_to_remove; ++i) {
        int row = indices[i] / 9;
        int col = indices[i] % 9;
        if (puzzle.grid[row][col] != 0) {
            puzzle.grid[row][col] = 0;
            removed_count++;
        }
    }

    return puzzle;
}
