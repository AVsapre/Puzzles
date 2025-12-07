#include "../include/puzzles/maze.h"
#include "../include/puzzles/rand.h"

Openings carve_openings(MazeGraph& g,
                        const bool carveEntrance,
                        const bool carveExit,
                        const int preferredEntrance,
                        const int preferredExit) {
    Openings openings;
    const int rows = g.rows;
    const int cols = g.cols;
    auto isBoundary = [&](int nodeId) {
        if (nodeId < 0 || nodeId >= static_cast<int>(g.nodes.size())) return false;
        const auto& n = g.nodes[nodeId];
        return n.row == 0 || n.col == 0 || n.row == rows - 1 || n.col == cols - 1;
    };

    auto randomBoundary = [&]() -> int {
        while (true) {
            const int r = rand_int(rows) - 1;
            const int c = rand_int(cols) - 1;
            if (r < 0 || c < 0) continue;
            if (r == 0 || c == 0 || r == rows - 1 || c == cols - 1) {
                return r * cols + c;
            }
        }
    };

    if (carveEntrance) {
        if (preferredEntrance >= 0 && preferredEntrance < static_cast<int>(g.nodes.size())) {
            
            openings.entranceNode = preferredEntrance;
        } else {
            openings.entranceNode = randomBoundary();
        }
        g.entranceNode = openings.entranceNode;
    }
    if (carveExit) {
        auto oppositeBoundary = [&]() -> int {
            if (openings.entranceNode < 0) return randomBoundary();
            const auto& n = g.nodes[openings.entranceNode];
            if (isBoundary(openings.entranceNode)) {
                if (n.row == 0) { 
                    const int c = rand_int(cols) - 1;
                    return (rows - 1) * cols + c;
                }
                if (n.row == rows - 1) { 
                    const int c = rand_int(cols) - 1;
                    return c;
                }
                if (n.col == 0) { 
                    const int r = rand_int(rows) - 1;
                    return r * cols + (cols - 1);
                }
                if (n.col == cols - 1) { 
                    const int r = rand_int(rows) - 1;
                    return r * cols;
                }
            }
            return randomBoundary();
        };

        if (preferredExit >= 0 && isBoundary(preferredExit) && preferredExit != openings.entranceNode) {
            openings.exitNode = preferredExit;
        } else {
            openings.exitNode = oppositeBoundary();
            if (openings.exitNode == openings.entranceNode) {
                openings.exitNode = randomBoundary();
            }
        }
        g.exitNode = openings.exitNode;
    }
    return openings;
}

MazeGraph build_maze_graph(const int units_width, const int units_height) {
    return make_grid_graph(units_height, units_width);
}
