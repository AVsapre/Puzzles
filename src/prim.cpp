#include "prim.h"

#include <algorithm>
#include <cassert>
#include <random>
#include <ranges>
#include <vector>

#include "algoutils.h"

namespace {
struct Frontier {
    int x;
    int y;
    int wx;
    int wy;
};
} // namespace

void prim(std::vector<std::vector<unsigned char>>& maze) {
    assert(!maze.empty() && !maze.front().empty());
    const int rows = static_cast<int>(maze.size());
    const int cols = static_cast<int>(maze.front().size());

    auto [startX, startY] = odd_center(cols, rows);
    maze[startY][startX] = 0;

    std::vector<Frontier> frontier;
    frontier.reserve(static_cast<size_t>(rows * cols) / 2);

    auto addFrontier = [&](const int x, const int y, const int wx, const int wy) {
        if (in_bounds_cell(x, y, cols, rows) && maze[y][x] == 1) {
            const bool exists = std::ranges::any_of(frontier, [&](const Frontier& f) {
                return f.x == x && f.y == y;
            });
            if (!exists) {
                frontier.push_back({x, y, wx, wy});
            }
        }
    };

    for (const auto& step : kSteps) {
        addFrontier(startX + step.dx, startY + step.dy, startX + step.dx / 2, startY + step.dy / 2);
    }

    while (!frontier.empty()) {
        const size_t idx = std::uniform_int_distribution<size_t>(0, frontier.size() - 1)(rng);
        Frontier f = frontier[idx];

        frontier.erase(
            std::remove_if(frontier.begin(), frontier.end(), [&](const Frontier& other) {
                return other.x == f.x && other.y == f.y;
            }),
            frontier.end()
        );

        if (maze[f.y][f.x] != 1) {
            continue;
        }

        maze[f.y][f.x] = 0;
        maze[f.wy][f.wx] = 0;

        for (const auto& step : kSteps) {
            addFrontier(f.x + step.dx, f.y + step.dy, f.x + step.dx / 2, f.y + step.dy / 2);
        }
    }
}
