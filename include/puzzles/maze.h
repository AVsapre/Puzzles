#pragma once

#include <vector>
#include <utility>
#include "maze_graph.h"

struct Openings {
    int entranceNode = -1;
    int exitNode = -1;
};

enum class Direction { UP, DOWN, LEFT, RIGHT };

Openings carve_openings(MazeGraph& g,
                        bool carveEntrance = true,
                        bool carveExit = true,
                        int preferredEntrance = -1,
                        int preferredExit = -1);

MazeGraph build_maze_graph(int units_width, int units_height);
