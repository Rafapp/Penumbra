#pragma once

#include <glm/glm.hpp>
#include <random>

class Sampler {
public:
    Sampler(uint32_t seed = 0);
    ~Sampler() = default;
    glm::vec3 SampleHemisphere();
    float Sample1D();

protected:
    std::mt19937 generator;
private:
};