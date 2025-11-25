#include "bfs.h"

#include <algorithm>
#include <cassert>
#include <queue>

#include "algoutils.h"

void bfs(std::vector<std::vector<unsigned char>> &v) {
    assert(!v.empty() && !v.front().empty());
    const int rows = static_cast<int>(v.size());
    const int cols = static_cast<int>(v.front().size());

    std::queue<std::pair<int, int>> q;
    auto [startX, startY] = odd_center(cols, rows);
    v[startY][startX] = 0;
    q.push({startX, startY});

    while (!q.empty()) {
        auto [cx, cy] = q.front();
        q.pop();

        auto dirs = kSteps;
        std::ranges::shuffle(dirs, rng);
        for (const auto& step : dirs) {
            if (carve_step(v, cx, cy, step)) {
                q.push({cx + step.dx, cy + step.dy});
            }
        }
    }
}
