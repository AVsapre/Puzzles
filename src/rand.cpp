#include "puzzles/rand.h"

int rand_int(const int max) {
    uniform_int_distribution dist(1, max);
    return dist(rng);
}