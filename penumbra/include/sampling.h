#pragma once
#pragma once

#define _USE_MATH_DEFINES
#include <cmath>

#ifndef M_PI 
#define M_PI 3.14159274101257324219f
#endif
#ifndef M_1_PI
#define M_1_PI 0.31830987334251403809f
#endif

#include <random>
#include <iostream>

#include <glm/glm.hpp>

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


