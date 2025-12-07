#pragma once
#ifndef PUZZLES_PRIM_H
#define PUZZLES_PRIM_H

#include "maze_graph.h"

MazeGraph prim_generate(MazeGraph g, int startNode = -1);

#endif 
