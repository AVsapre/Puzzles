#include "puzzles/algoutils.h"

#include <algorithm>
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

MazeGraph prim_generate(MazeGraph g, const int startNode) {
    const int count = static_cast<int>(g.nodes.size());
    if (count == 0) return g;

    auto adj = buildAdj(g);
    const int start = pickStart(startNode, count);

    std::vector<bool> inTree(count, false);
    std::vector<Adjacent> frontier;

    auto addFrontier = [&](int node) {
        for (const auto& nb : adj[node]) {
            if (!inTree[nb.to]) {
                frontier.push_back(nb);
            }
        }
    };

    inTree[start] = true;
    addFrontier(start);

    while (!frontier.empty()) {
        std::shuffle(frontier.begin(), frontier.end(), rng);
        const Adjacent chosen = frontier.back();
        frontier.pop_back();
        if (inTree[chosen.to]) {
            continue;
        }
        g.edges[chosen.edge].open = true;
        inTree[chosen.to] = true;
        addFrontier(chosen.to);
    }

    return g;
}
