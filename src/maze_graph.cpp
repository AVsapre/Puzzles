#include "../include/puzzles/maze_graph.h"

MazeGraph make_grid_graph(const int rows, const int cols) {
    MazeGraph g;
    g.rows = rows;
    g.cols = cols;
    g.nodes.reserve(rows * cols);
    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            const int id = static_cast<int>(g.nodes.size());
            g.nodes.push_back(MazeNode{id, r, c});
        }
    }
    auto nodeId = [cols](int r, int c) { return r * cols + c; };
    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            const int id = nodeId(r, c);
            if (c + 1 < cols) {
                g.edges.push_back(MazeEdge{id, nodeId(r, c + 1), false});
            }
            if (r + 1 < rows) {
                g.edges.push_back(MazeEdge{id, nodeId(r + 1, c), false});
            }
        }
    }
    return g;
}

int edge_index_between(const MazeGraph& g, const int from, const int to) {
    for (int i = 0; i < static_cast<int>(g.edges.size()); ++i) {
        const auto& e = g.edges[i];
        if ((e.from == from && e.to == to) || (e.from == to && e.to == from)) {
            return i;
        }
    }
    return -1;
}
