#pragma once

#define _USE_MATH_DEFINES
#include <cmath>

#ifndef M_PI 
#define M_PI 3.14159274101257324219f
#endif
#ifndef M_1_PI
#define M_1_PI 0.31830987334251403809f
#endif

#include "glm/glm.hpp"
#include "sampling.h"
#include "image.h"

class Texture{
public:
    Texture();
    ~Texture();
    bool Load(const std::string& path);
    glm::vec3 Sample(const glm::vec2& uv) const;

private:
    std::string path;
    std::vector<float> pixels;
    int xres = 0;
    int yres = 0;
    int nchannels = 0;
};