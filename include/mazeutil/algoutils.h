#pragma once

#include <array>
#include <utility>
#include <vector>

#include "maze.h"
#include "rand.h"

struct Step {
    Direction dir;
    int dx;
    int dy;
};

inline constexpr std::array<Step, 4> kSteps = {{
    {Direction::UP, 0, -2},
    {Direction::DOWN, 0, 2},
    {Direction::LEFT, -2, 0},
    {Direction::RIGHT, 2, 0},
}};

inline std::pair<int, int> odd_center(const int cols, const int rows) {
    int cx = cols / 2;
    int cy = rows / 2;
    if ((cx & 1) == 0) { ++cx; }
    if ((cy & 1) == 0) { ++cy; }
    return {cx, cy};
}

inline bool in_bounds_cell(const int x, const int y, const int cols, const int rows) {
    return x > 0 && y > 0 && x < cols - 1 && y < rows - 1;
}

inline bool carve_step(std::vector<std::vector<unsigned char>>& grid, const int x, const int y, const Step& step) {
    const int rows = static_cast<int>(grid.size());
    const int cols = static_cast<int>(grid.front().size());
    const int nx = x + step.dx;
    const int ny = y + step.dy;

    if (!in_bounds_cell(nx, ny, cols, rows) || grid[ny][nx] != 1) {
        return false;
    }

    grid[y + step.dy / 2][x + step.dx / 2] = 0;
    grid[ny][nx] = 0;
    return true;
}
