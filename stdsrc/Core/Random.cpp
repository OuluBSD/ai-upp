#include "Core.h"
#include <random>
#include <ctime>

NAMESPACE_UPP

// Global random number generator
static std::mt19937 global_gen;
static bool global_gen_seeded = false;

void SeedRandom() {
    global_gen.seed(static_cast<unsigned int>(time(nullptr)));
    global_gen_seeded = true;
}

void SeedRandom(unsigned int seed) {
    global_gen.seed(seed);
    global_gen_seeded = true;
}

int Random() {
    if (!global_gen_seeded) {
        SeedRandom();
    }
    return static_cast<int>(global_gen());
}

int Random(int max) {
    if (!global_gen_seeded) {
        SeedRandom();
    }
    if (max <= 0) return 0;
    return static_cast<int>(global_gen()) % max;
}

int Random(int min, int max) {
    if (!global_gen_seeded) {
        SeedRandom();
    }
    if (min >= max) return min;
    std::uniform_int_distribution<int> dist(min, max - 1);
    return dist(global_gen);
}

double Randomf() {
    if (!global_gen_seeded) {
        SeedRandom();
    }
    std::uniform_real_distribution<double> dist(0.0, 1.0);
    return dist(global_gen);
}

double Randomf(double max) {
    if (!global_gen_seeded) {
        SeedRandom();
    }
    if (max <= 0.0) return 0.0;
    std::uniform_real_distribution<double> dist(0.0, max);
    return dist(global_gen);
}

double Randomf(double min, double max) {
    if (!global_gen_seeded) {
        SeedRandom();
    }
    if (min >= max) return min;
    std::uniform_real_distribution<double> dist(min, max);
    return dist(global_gen);
}

int64 Random64() {
    if (!global_gen_seeded) {
        SeedRandom();
    }
    static std::uniform_int_distribution<int64> dist;
    return dist(global_gen);
}

int64 Random64(int64 max) {
    if (!global_gen_seeded) {
        SeedRandom();
    }
    if (max <= 0) return 0;
    std::uniform_int_distribution<int64> dist(0, max - 1);
    return dist(global_gen);
}

int64 Random64(int64 min, int64 max) {
    if (!global_gen_seeded) {
        SeedRandom();
    }
    if (min >= max) return min;
    std::uniform_int_distribution<int64> dist(min, max - 1);
    return dist(global_gen);
}

END_UPP_NAMESPACE