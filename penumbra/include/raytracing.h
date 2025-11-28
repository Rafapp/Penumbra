#pragma once

#include "glm/glm.hpp"

struct Ray{
    glm::vec3 o;
    glm::vec3 d;
    Ray() = default;
    Ray( glm::vec3 const &_o, glm::vec3 const &_d ) : o(_o), d(glm::normalize(_d)) {}
    glm::vec3 At(float t) const {
        return o + t * d;
    }
};

struct HitInfo{
    float t;
    glm::vec3 p;
    glm::vec3 n;
};