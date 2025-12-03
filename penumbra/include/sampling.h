#pragma once

#include <glm/glm.hpp>
#include <random>

class Sampler {
public:
    Sampler(uint32_t seed = 0);
    ~Sampler() = default;
    glm::vec3 SampleHemisphere();
    float Sample1D();
    int SampleInt(int min, int max);
    glm::vec3 SampleHemisphereUniform(const glm::vec3& normal);
    glm::vec3 SampleHemisphereCosine(const glm::vec3& normal);
    glm::vec2 SampleUnitDisk();

protected:
    std::mt19937 generator;
private:
};


