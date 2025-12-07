#pragma once

#include <random>
#include "maze_graph.h"
#include "rand.h"


extern std::mt19937 rng;
int rand_int(int bound); 


MazeGraph dfs_generate(MazeGraph g, int startNode = -1);
MazeGraph bfs_generate(MazeGraph g, int startNode = -1);
MazeGraph prim_generate(MazeGraph g, int startNode = -1);
MazeGraph kruskal_generate(MazeGraph g);
MazeGraph wilson_generate(MazeGraph g, int startNode = -1);
