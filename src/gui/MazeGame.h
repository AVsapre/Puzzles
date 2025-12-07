#pragma once

#include <utility>
#include <vector>

#include "maze.h"
#include "../include/puzzles/maze_graph.h"

enum class GenerationAlgorithm { DFS, BFS, Wilson, Kruskal, Prim, Tessellation };

class MazeGame {
public:
    bool generate(GenerationAlgorithm algorithm,
                  int widthUnits,
                  int heightUnits,
                  std::pair<int, int> startUnits = {-1, -1},
                  std::pair<int, int> exitUnits = {-1, -1},
                  bool customExit = false);
    bool move(Direction direction);
    void load(const MazeGraph& graph,
              int entranceNode,
              int exitNode,
              int playerNode = -1);

    [[nodiscard]] const MazeGraph& graph() const { return graph_; }
    [[nodiscard]] bool tested() const { return tested_; }
    [[nodiscard]] bool hasMaze() const { return !graph_.nodes.empty(); }
    [[nodiscard]] int rows() const { return graph_.rows; }
    [[nodiscard]] int cols() const { return graph_.cols; }
    [[nodiscard]] int entranceNode() const { return entranceNode_; }
    [[nodiscard]] int exitNode() const { return exitNode_; }
    [[nodiscard]] int playerNode() const { return playerNode_; }
    [[nodiscard]] std::pair<int, int> entranceCell() const;
    [[nodiscard]] std::pair<int, int> exitCell() const;
    [[nodiscard]] std::pair<int, int> playerCell() const;

private:
    MazeGraph graph_;
    int entranceNode_ = -1;
    int exitNode_ = -1;
    int playerNode_ = -1;
    bool tested_ = false;

    void refreshMarkers();
    [[nodiscard]] std::pair<int, int> nodeCoords(int node) const;
};
