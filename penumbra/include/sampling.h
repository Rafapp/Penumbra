#pragma once

#define _USE_MATH_DEFINES
#include <cmath>

#include <glm/glm.hpp>
#include <random>

#include "utils.h"

class Sampler {
public:
    Sampler(uint32_t seed = 0);
    ~Sampler() = default;
    float Sample1D();
    int SampleInt(int min, int max);
    glm::vec2 SampleHalton2D(uint32_t b1, uint32_t b2, uint32_t index);
    glm::vec3 SampleSphereUniform();
    glm::vec3 SampleHemisphereUniform(const glm::vec3& n);
    glm::vec3 SampleHemisphereCosine(const glm::vec3& n);
    glm::vec3 SampleHemisphereGGX(const glm::vec3& n, float r);
    glm::vec2 SampleUnitDiskUniform();

protected:
    std::mt19937 generator;
private:
};


