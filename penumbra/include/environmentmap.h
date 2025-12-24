#pragma once

#define _USE_MATH_DEFINES
#include <cmath>

#ifndef M_PI 
#define M_PI 3.14159274101257324219f
#endif
#ifndef M_1_PI
#define M_1_PI 0.31830987334251403809f
#endif

#include <image.h>
#include "glm/glm.hpp"
#include "materials.h"
#include "sampling.h"

class EnvironmentMap {
public:
    EnvironmentMap(std::string filepath);
    ~EnvironmentMap();
    bool Load();
private:
    std::string filepath;
};