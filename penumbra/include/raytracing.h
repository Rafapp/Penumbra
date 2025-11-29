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
    Ray Transform(const glm::mat4& _m) const {
        glm::vec4 oNew = _m * glm::vec4(o.x, o.y, o.z, 1.0f);
        glm::vec4 dNew = _m * glm::vec4(d.x, d.y, d.z, 1.0f);
        return Ray(glm::vec3(oNew) / oNew.w, glm::vec3(dNew));
    }
};

struct HitInfo{
    float t;
    glm::vec3 p;
    glm::vec3 n;
    bool front;
    int materialId = -1;
};