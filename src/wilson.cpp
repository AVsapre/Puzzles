#include "wilson.h"
#include <algorithm>
#include <array>
#include <cassert>
#include <random>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "algoutils.h"
#include "maze.h"
#include "rand.h"

static uint64_t encode(const int x, const int y) {
    return (static_cast<uint64_t>(y) << 32) | static_cast<uint32_t>(x);
}

static void carvePath(
    std::vector<std::vector<unsigned char>>& v,
    const std::vector<std::pair<int,int>>& path,
    std::unordered_set<uint64_t>& unvisited
) {
    for (size_t i = 0; i + 1 < path.size(); ++i) {
        auto a = path[i];
        auto b = path[i + 1];
        v[a.second][a.first] = 0;
        v[b.second][b.first] = 0;
        v[(a.second + b.second) / 2][(a.first + b.first) / 2] = 0;
        unvisited.erase(encode(a.first, a.second));
        unvisited.erase(encode(b.first, b.second));
    }
}

void wilson(std::vector<std::vector<unsigned char>>& v) {
    assert(!v.empty() && !v.front().empty());
    const int height = v.size();
    const int width = v[0].size();

    std::unordered_set<uint64_t> unvisited;
    for (int y = 1; y < height; y += 2)
        for (int x = 1; x < width; x += 2)
            unvisited.insert(encode(x, y));

    std::uniform_int_distribution dist_x(1, width - 2);
    std::uniform_int_distribution dist_y(1, height - 2);
    int sx = dist_x(rng) | 1;
    int sy = dist_y(rng) | 1;
    v[sy][sx] = 0;
    unvisited.erase(encode(sx, sy));

    auto pickRandomUnvisited = [&]() -> pair<int,int> {
        const size_t idx = uniform_int_distribution<size_t>(0, unvisited.size() - 1)(rng);
        auto it = unvisited.begin();
        advance(it, idx);
        const uint64_t key = *it;
        return {static_cast<int>(key & 0xffffffffu), static_cast<int>(key >> 32)};
    };

    while (!unvisited.empty()) {
        const auto cur = pickRandomUnvisited();
        std::vector path = {cur};
        std::unordered_map<uint64_t, size_t> index;
        index[encode(cur.first, cur.second)] = 0;

        while (true) {
            std::vector<Step> validMoves;
            for (const auto& step : kSteps) {
                const int nx = path.back().first + step.dx;
                const int ny = path.back().second + step.dy;
                if (in_bounds_cell(nx, ny, width, height) && (nx & 1) && (ny & 1)) {
                    validMoves.push_back(step);
                }
            }

            if (validMoves.empty())
                break;

            const Step step = validMoves[std::uniform_int_distribution<size_t>(0, validMoves.size() - 1)(rng)];
            const auto next = std::pair<int, int>{
                path.back().first + step.dx,
                path.back().second + step.dy
            };

            if (uint64_t key = encode(next.first, next.second); index.contains(key)) {
                path.resize(index[key] + 1);
            } else {
                path.push_back(next);
                index[key] = path.size() - 1;
            }

            if (v[next.second][next.first] == 0) {
                carvePath(v, path, unvisited);
                break;
            }
        }
    }
}
