#include "puzzles/algoutils.h"

#include <algorithm>
#include <numeric>
#include <vector>

namespace {
struct DisjointSet {
    explicit DisjointSet(const int n) : parent(n), rank(n, 0) {
        std::iota(parent.begin(), parent.end(), 0);
    }
    int find(const int x) {
        if (parent[x] != x) parent[x] = find(parent[x]);
        return parent[x];
    }
    bool unite(const int a, const int b) {
        int ra = find(a);
        int rb = find(b);
        if (ra == rb) return false;
        if (rank[ra] < rank[rb]) std::swap(ra, rb);
        parent[rb] = ra;
        if (rank[ra] == rank[rb]) ++rank[ra];
        return true;
    }
    std::vector<int> parent;
    std::vector<int> rank;
};
} 

MazeGraph kruskal_generate(MazeGraph g) {
    const int edgeCount = static_cast<int>(g.edges.size());
    if (edgeCount == 0) return g;

    std::vector<int> order(edgeCount);
    std::iota(order.begin(), order.end(), 0);
    std::shuffle(order.begin(), order.end(), rng);

    DisjointSet ds(static_cast<int>(g.nodes.size()));
    for (const int idx : order) {
        const auto& e = g.edges[idx];
        if (ds.unite(e.from, e.to)) {
            g.edges[idx].open = true;
        }
    }

    return g;
}
