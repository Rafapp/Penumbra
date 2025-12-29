#pragma once

#include "glm/glm.hpp"

class AreaLight;
class Shape;
class Material;

struct Ray{
    glm::vec3 o;
    glm::vec3 d;
    Ray() = default;
    Ray( glm::vec3 const &o, glm::vec3 const &d ) : o(o), d(glm::normalize(d)) {}
    glm::vec3 At(float t) const {
        return o + t * d;
    }
    Ray Transform(const glm::mat4& m) const {
        glm::vec4 oNew = m * glm::vec4(o.x, o.y, o.z, 1.0f);
        glm::vec4 dNew = m * glm::vec4(d.x, d.y, d.z, 0.0f);
        return Ray(glm::vec3(oNew) / oNew.w, glm::normalize(glm::vec3(dNew)));
    }
};

struct HitInfo{
    float t = FLT_MAX;
    glm::vec3 p = glm::vec3(0.0f);
    glm::vec3 n = glm::vec3(0.0f);
    bool front = false;
    int materialId = -1;
    int areaLightId = -1;
    AreaLight* areaLight = nullptr;
    Shape* shape = nullptr;
    Material* material = nullptr;
};