#pragma once

#include <vector>
#include <utility>

struct MazeNode {
    int id = -1;
    int row = 0;
    int col = 0;
};

struct MazeEdge {
    int from = -1;
    int to = -1;
    bool open = false;
};

struct MazeGraph {
    int rows = 0; 
    int cols = 0; 
    int entranceNode = -1;
    int exitNode = -1;
    std::vector<MazeNode> nodes;
    std::vector<MazeEdge> edges;
};


MazeGraph make_grid_graph(int rows, int cols);


int edge_index_between(const MazeGraph& g, int from, int to);
