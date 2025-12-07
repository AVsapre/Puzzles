#include "puzzles/algoutils.h"

#include <algorithm>
#include <stack>
#include <vector>

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

MazeGraph dfs_generate(MazeGraph g, const int startNode) {
    const int count = static_cast<int>(g.nodes.size());
    if (count == 0) return g;

    const int start = pickStart(startNode, count);
    auto adj = buildAdj(g);

    std::vector<bool> visited(count, false);
    std::stack<int> stack;
    visited[start] = true;
    stack.push(start);

    while (!stack.empty()) {
        const int current = stack.top();
        std::vector<Adjacent> neighbors = adj[current];
        std::shuffle(neighbors.begin(), neighbors.end(), rng);

        bool advanced = false;
        for (const auto& nb : neighbors) {
            if (visited[nb.to]) continue;
            g.edges[nb.edge].open = true;
            visited[nb.to] = true;
            stack.push(nb.to);
            advanced = true;
            break;
        }
        if (!advanced) {
            stack.pop();
        }
    }

    return g;
}
