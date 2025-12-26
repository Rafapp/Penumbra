#pragma once

#define _USE_MATH_DEFINES
#include <cmath>

#ifndef M_PI 
#define M_PI 3.14159274101257324219f
#endif
#ifndef M_1_PI
#define M_1_PI 0.31830987334251403809f
#endif
#ifndef M_1_2PI
#define M_1_2PI 0.15915493667125701904f
#endif

#include "glm/glm.hpp"

#include "raytracing.h"
#include "image.h"
#include "materials.h"
#include "sampling.h"

class EnvironmentMap {
public:
    EnvironmentMap() = default;
    EnvironmentMap(std::string filepath);
    ~EnvironmentMap();
    bool Load();
    glm::vec3 SampleColor(const Ray& ray);
private:
    std::string filepath;
    std::vector<uint8_t> image;
    int width;
    int height;
    int nChannels;
};