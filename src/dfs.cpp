#include "dfs.h"

#include <algorithm>
#include <cassert>
#include <stack>

#include "algoutils.h"

namespace {
bool step_from(std::vector<std::vector<unsigned char>>& grid,
               const int x,
               const int y,
               const std::array<Step, 4>& dirs,
               std::stack<std::pair<int, int>>& stack) {
    auto shuffled = dirs;
    std::ranges::shuffle(shuffled, rng);

    for (const auto& step : shuffled) {
        if (carve_step(grid, x, y, step)) {
            stack.push({x + step.dx, y + step.dy});
            return true;
        }
    }
    return false;
}
} // namespace

void dfs(std::vector<std::vector<unsigned char>> &v) {
    assert(!v.empty() && !v.front().empty());
    const int rows = static_cast<int>(v.size());
    const int cols = static_cast<int>(v.front().size());

    auto [startX, startY] = odd_center(cols, rows);
    v[startY][startX] = 0;

    std::stack<std::pair<int, int>> stack;
    stack.push({startX, startY});

    while (!stack.empty()) {
        auto [x, y] = stack.top();
        if (!step_from(v, x, y, kSteps, stack)) {
            stack.pop();
        }
    }
}
