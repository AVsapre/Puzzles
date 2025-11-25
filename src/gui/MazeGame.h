#pragma once

#include <utility>
#include <vector>
#include <tuple>

#include "maze.h"

enum class GenerationAlgorithm { DFS, BFS, Wilson, Kruskal, Prim, Tessellation };

class MazeGame {
public:
    void generate(GenerationAlgorithm algorithm, int widthUnits, int heightUnits, int depthUnits);
    bool move(Direction direction);
    bool moveLayer(int delta);
    void load(const std::vector<std::vector<std::vector<unsigned char>>>& grid,
              std::tuple<int, int, int> start,
              std::tuple<int, int, int> goal);

    [[nodiscard]] const std::vector<std::vector<std::vector<unsigned char>>>& grid() const { return grid_; }
    [[nodiscard]] bool solved() const { return solved_; }
    [[nodiscard]] bool hasMaze() const { return !grid_.empty(); }
    [[nodiscard]] int rows() const { return grid_.empty() ? 0 : static_cast<int>(grid_.front().size()); }
    [[nodiscard]] int cols() const { return (grid_.empty() || grid_.front().empty()) ? 0 : static_cast<int>(grid_.front().front().size()); }
    [[nodiscard]] int layers() const { return static_cast<int>(grid_.size()); }
    [[nodiscard]] int currentLayer() const { return layer_; }
    [[nodiscard]] const std::vector<std::vector<unsigned char>>& layerGrid(int layer) const { return grid_.at(layer); }
    [[nodiscard]] std::tuple<int, int, int> start() const { return start_; }
    [[nodiscard]] std::tuple<int, int, int> goal() const { return goal_; }
    [[nodiscard]] std::tuple<int, int, int> player() const { return player_; }

private:
    std::vector<std::vector<std::vector<unsigned char>>> grid_;
    std::tuple<int, int, int> player_{0, 0, 0}; // layer, row, col
    std::tuple<int, int, int> start_{0, 0, 0};
    std::tuple<int, int, int> goal_{0, 0, 0};
    int layer_ = 0;
    bool solved_ = false;

    void refreshMarkers();
    void placeVerticalConnectors(int connectorsPerLayer);
};
