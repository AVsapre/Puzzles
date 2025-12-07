#include "puzzles/algoutils.h"

#include <algorithm>
#include <queue>
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

MazeGraph bfs_generate(MazeGraph g, const int startNode) {
    const int count = static_cast<int>(g.nodes.size());
    if (count == 0) return g;

    auto adj = buildAdj(g);
    const int start = pickStart(startNode, count);

    std::vector<bool> visited(count, false);
    std::queue<int> q;
    visited[start] = true;
    q.push(start);

    while (!q.empty()) {
        const int current = q.front();
        q.pop();
        auto neighbors = adj[current];
        std::shuffle(neighbors.begin(), neighbors.end(), rng);
        for (const auto& nb : neighbors) {
            if (visited[nb.to]) continue;
            visited[nb.to] = true;
            g.edges[nb.edge].open = true;
            q.push(nb.to);
        }
    }

    return g;
}
