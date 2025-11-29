#pragma once

#include "glm/glm.hpp"
#include "minipbrt.h"

class Light {
public:
    virtual ~Light() = default;
    
    glm::vec3 GetPosition() const { return position; }
    glm::vec3 GetIntensity() const { return intensity; }
    
protected:
    glm::vec3 position;
    glm::vec3 intensity;
};

class PointLight : public Light {
public:
    PointLight(minipbrt::PointLight* pbrtLight);
    ~PointLight() = default;
};