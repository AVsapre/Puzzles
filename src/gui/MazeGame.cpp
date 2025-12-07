#include "MazeGame.h"

#include "../include/puzzles/maze.h"
#include "../include/puzzles/maze_graph.h"
#include "puzzles/algoutils.h"

namespace {
int clampUnits(const int value) {
    return std::max(2, value);
}
}

bool MazeGame::generate(const GenerationAlgorithm algorithm,
                        const int widthUnits,
                        const int heightUnits,
                        const std::pair<int, int> startUnits,
                        const std::pair<int, int> exitUnits,
                        const bool customExit) {
    tested_ = false;
    graph_ = build_maze_graph(clampUnits(widthUnits), clampUnits(heightUnits));

    auto nodeIdFromUnits = [&](std::pair<int, int> units) -> int {
        const int r = units.second - 1;
        const int c = units.first - 1;
        if (r < 0 || c < 0 || r >= graph_.rows || c >= graph_.cols) return -1;
        return r * graph_.cols + c;
    };

    int startNode = nodeIdFromUnits(startUnits);
    int exitNode = customExit ? nodeIdFromUnits(exitUnits) : -1;

    switch (algorithm) {
        case GenerationAlgorithm::DFS: graph_ = dfs_generate(std::move(graph_), startNode); break;
        case GenerationAlgorithm::BFS: graph_ = bfs_generate(std::move(graph_), startNode); break;
        case GenerationAlgorithm::Wilson: graph_ = wilson_generate(std::move(graph_), startNode); break;
        case GenerationAlgorithm::Kruskal: graph_ = kruskal_generate(std::move(graph_)); break;
        case GenerationAlgorithm::Prim: graph_ = prim_generate(std::move(graph_), startNode); break;
        case GenerationAlgorithm::Tessellation: graph_ = kruskal_generate(std::move(graph_)); break;
    }

    const Openings openings = carve_openings(graph_, true, true, startNode, exitNode);
    entranceNode_ = openings.entranceNode;
    exitNode_ = openings.exitNode;
    playerNode_ = entranceNode_;
    refreshMarkers();
    return true;
}

bool MazeGame::move(const Direction direction) {
    if (playerNode_ < 0) return false;
    const auto [row, col] = nodeCoords(playerNode_);
    if (row < 0 || col < 0) return false;

    int nextRow = row;
    int nextCol = col;
    switch (direction) {
        case Direction::UP:    --nextRow; break;
        case Direction::DOWN:  ++nextRow; break;
        case Direction::LEFT:  --nextCol; break;
        case Direction::RIGHT: ++nextCol; break;
    }

    if (nextRow < 0 || nextCol < 0 || nextRow >= graph_.rows || nextCol >= graph_.cols) {
        return false;
    }

    const int nextNode = nextRow * graph_.cols + nextCol;
    const int edgeIdx = edge_index_between(graph_, playerNode_, nextNode);
    if (edgeIdx < 0 || !graph_.edges[edgeIdx].open) {
        return false;
    }

    playerNode_ = nextNode;
    tested_ = true;
    return true;
}

void MazeGame::load(const MazeGraph& graph,
                    const int entranceNode,
                    const int exitNode,
                    const int playerNode) {
    graph_ = graph;
    entranceNode_ = entranceNode;
    exitNode_ = exitNode;
    playerNode_ = playerNode >= 0 ? playerNode : entranceNode_;
    tested_ = false;
    refreshMarkers();
}

void MazeGame::refreshMarkers() {
    
}

std::pair<int, int> MazeGame::nodeCoords(const int node) const {
    if (node < 0 || node >= static_cast<int>(graph_.nodes.size())) return {-1, -1};
    const auto& n = graph_.nodes[node];
    return {n.row, n.col};
}

std::pair<int, int> MazeGame::entranceCell() const {
    return nodeCoords(entranceNode_);
}

std::pair<int, int> MazeGame::exitCell() const {
    return nodeCoords(exitNode_);
}

std::pair<int, int> MazeGame::playerCell() const {
    return nodeCoords(playerNode_);
}
