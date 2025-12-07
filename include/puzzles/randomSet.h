#pragma once
#include <vector>
#include <unordered_map>
#include <random>
#include <utility>
#include <cassert>



struct pair_hash {
    template <typename T1, typename T2>
    std::size_t operator()(const std::pair<T1,T2>& p) const noexcept {
        auto h1 = std::hash<T1>{}(p.first);
        auto h2 = std::hash<T2>{}(p.second);
        return h1 ^ (h2 << 1);
    }
};

template<typename T, typename Hash = std::hash<T>>
class RandomAccessSet {
    std::vector<T> items;
    std::unordered_map<T, size_t, Hash> indices;
    mutable std::mt19937 rng;  

public:
    RandomAccessSet()
        : rng(std::random_device{}()) {}

    bool insert(const T& item) {
        if (indices.count(item)) return false;
        indices[item] = items.size();
        items.push_back(item);
        return true;
    }

    bool remove(const T& item) {
        auto it = indices.find(item);
        if (it == indices.end()) return false;

        size_t idx = it->second;
        if (idx != items.size() - 1) {
            std::swap(items[idx], items.back());
            indices[items[idx]] = idx;
        }

        items.pop_back();
        indices.erase(it);
        return true;
    }

    bool contains(const T& item) const {
        return indices.count(item) > 0;
    }

    bool empty() const { return items.empty(); }
    size_t size() const { return items.size(); }

    T getRandom() const {
        assert(!items.empty());
        std::uniform_int_distribution<size_t> dist(0, items.size() - 1);
        return items[dist(rng)];
    }
};
