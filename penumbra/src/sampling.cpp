#include "sampling.h"

Sampler::Sampler(uint32_t seed) {
    generator.seed(seed);
}

float Sampler::Sample1D() {
    std::uniform_real_distribution<float> distribution(0.0f, 1.0f);
    return distribution(generator);
}