#include "puzzles/algoutils.h"

#include <algorithm>
#include <unordered_map>
#include <vector>
#include <limits>

namespace {
struct Adjacent {
    int to;
    int edge;
};

std::vector<std::vector<Adjacent>> buildAdj(const MazeGraph& g) {
    std::vector<std::vector<Adjacent>> adj(g.nodes.size());
    for (int i = 0; i < static_cast<int>(g.edges.size()); ++i) {
        const auto& e = g.edges[i];
        adj[e.from].push_back({e.to, i});
        adj[e.to].push_back({e.from, i});
    }
    return adj;
}

int pickStart(const int requested, const int count) {
    if (requested >= 0 && requested < count) return requested;
    return rand_int(count) - 1;
}
} 

MazeGraph wilson_generate(MazeGraph g, const int startNode) {
    const int count = static_cast<int>(g.nodes.size());
    if (count == 0) return g;

    auto adj = buildAdj(g);
    std::vector<bool> inTree(count, false);
    std::vector<int> visitedIndex(count, -1); 

    const int root = pickStart(startNode, count);
    inTree[root] = true;

    for (int start = 0; start < count; ++start) {
        if (inTree[start]) continue;

        std::vector<int> path;
        int current = start;
        path.push_back(current);
        visitedIndex[current] = 0;

        
        while (!inTree[current]) {
            auto neighbors = adj[current];
            std::shuffle(neighbors.begin(), neighbors.end(), rng);
            const int next = neighbors.front().to;
            current = next;

            if (visitedIndex[current] != -1) {
                
                const int loopStart = visitedIndex[current];
                
                for (int i = loopStart + 1; i < static_cast<int>(path.size()); ++i) {
                    visitedIndex[path[i]] = -1;
                }
                path.resize(loopStart + 1);
            } else {
                visitedIndex[current] = static_cast<int>(path.size());
                path.push_back(current);
            }
        }

        
        for (int i = 0; i + 1 < static_cast<int>(path.size()); ++i) {
            const int a = path[i];
            const int b = path[i + 1];
            const int edgeIdx = edge_index_between(g, a, b);
            if (edgeIdx >= 0) {
                g.edges[edgeIdx].open = true;
            }
            inTree[a] = true;
        }
        
        inTree[path.back()] = true;

        
        for (int node : path) {
            visitedIndex[node] = -1;
        }
    }

    return g;
}
