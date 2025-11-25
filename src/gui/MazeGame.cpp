#include "MazeGame.h"

#include <algorithm>
#include <random>
#include <tuple>

#include "algoutils.h"
#include "bfs.h"
#include "dfs.h"
#include "kruskal.h"
#include "prim.h"
#include "tessellate.h"
#include "wilson.h"

extern std::vector<std::vector<unsigned char>> maze;
extern std::pair<int, int> maze_in;
extern std::pair<int, int> maze_out;

namespace {
int clampUnits(const int value) {
    return std::max(2, value);
}
} // namespace

void MazeGame::generate(const GenerationAlgorithm algorithm, const int widthUnits, const int heightUnits, const int depthUnits) {
    solved_ = false;
    layer_ = 0;
    grid_.clear();

    const int depth = std::max(1, depthUnits);

    auto generateLayer = [&](std::vector<std::vector<unsigned char>>& layer) {
        if (algorithm == GenerationAlgorithm::Tessellation) {
            tessellate(std::max(1, widthUnits));
            layer = maze;
        } else {
            layer = maze_template(clampUnits(widthUnits), clampUnits(heightUnits));
            switch (algorithm) {
                case GenerationAlgorithm::DFS: dfs(layer); break;
                case GenerationAlgorithm::BFS: bfs(layer); break;
                case GenerationAlgorithm::Wilson: wilson(layer); break;
                case GenerationAlgorithm::Kruskal: kruskal(layer); break;
                case GenerationAlgorithm::Prim: prim(layer); break;
                case GenerationAlgorithm::Tessellation: break;
            }
        }
    };

    grid_.resize(depth);
    for (int z = 0; z < depth; ++z) {
        generateLayer(grid_[z]);
    }

    carve_openings(grid_.front());
    start_ = std::make_tuple(0, maze_in.first, maze_in.second);

    carve_openings(grid_.back());
    goal_ = std::make_tuple(depth - 1, maze_out.first, maze_out.second);

    placeVerticalConnectors(std::max(1, std::min(widthUnits, heightUnits) / 4));

    player_ = start_;
    refreshMarkers();
}

bool MazeGame::move(const Direction direction) {
    if (grid_.empty() || solved_) {
        return false;
    }

    int l, r, c;
    std::tie(l, r, c) = player_;
    int newRow = r;
    int newCol = c;

    switch (direction) {
        case Direction::UP:    newRow -= 1; break;
        case Direction::DOWN:  newRow += 1; break;
        case Direction::LEFT:  newCol -= 1; break;
        case Direction::RIGHT: newCol += 1; break;
    }

    if (newRow < 0 || newCol < 0 || newRow >= rows() || newCol >= cols()) {
        return false;
    }

    const unsigned char cell = grid_[l][newRow][newCol];
    if (cell == 1 || cell == 4) {
        return false;
    }

    grid_[l][r][c] = 0;
    player_ = {l, newRow, newCol};

    if (player_ == goal_) {
        solved_ = true;
    }

    refreshMarkers();
    return true;
}

bool MazeGame::moveLayer(const int delta) {
    if (grid_.empty() || solved_ || delta == 0) {
        return false;
    }

    int l, r, c;
    std::tie(l, r, c) = player_;
    const int target = l + delta;
    if (target < 0 || target >= static_cast<int>(grid_.size())) {
        return false;
    }

    const unsigned char cell = grid_[l][r][c];
    if ((delta > 0 && cell != 5) || (delta < 0 && cell != 6)) {
        return false;
    }

    if (grid_[target][r][c] == 1 || grid_[target][r][c] == 4) {
        return false;
    }

    grid_[l][r][c] = 0;
    layer_ = target;
    player_ = {target, r, c};

    if (player_ == goal_) {
        solved_ = true;
    }

    refreshMarkers();
    return true;
}

void MazeGame::load(const std::vector<std::vector<std::vector<unsigned char>>>& grid,
                    std::tuple<int, int, int> start,
                    std::tuple<int, int, int> goal) {
    grid_ = grid;
    start_ = start;
    goal_ = goal;
    player_ = start;
    layer_ = std::get<0>(start);
    solved_ = false;
    refreshMarkers();
}

void MazeGame::refreshMarkers() {
    if (grid_.empty()) {
        return;
    }

    for (auto& layer : grid_) {
        for (auto& row : layer) {
            for (auto& cell : row) {
                if (cell == 2 || cell == 3) {
                    cell = 0;
                }
            }
        }
    }

    const auto [l, r, c] = player_;
    grid_[l][r][c] = 2;
    if (!solved_) {
        const auto [gl, gr, gc] = goal_;
        grid_[gl][gr][gc] = 3;
    }
}

void MazeGame::placeVerticalConnectors(const int connectorsPerLayer) {
    if (grid_.size() < 2) {
        return;
    }

    std::uniform_int_distribution<int> distRow(1, rows() - 2);
    std::uniform_int_distribution<int> distCol(1, cols() - 2);

    for (size_t z = 0; z + 1 < grid_.size(); ++z) {
        for (int i = 0; i < connectorsPerLayer; ++i) {
            const int r = distRow(rng) | 1;
            const int c = distCol(rng) | 1;
            const int goalLayer = std::get<0>(goal_);
            if ((z == 0 && r == std::get<1>(start_) && c == std::get<2>(start_)) ||
                (static_cast<int>(z + 1) == goalLayer && r == std::get<1>(goal_) && c == std::get<2>(goal_))) {
                continue;
            }
            grid_[z][r][c] = 5;       // down marker
            grid_[z + 1][r][c] = 6;   // up marker
        }
    }
}
