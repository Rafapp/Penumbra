#pragma once

#include "glm/glm.hpp"
#include "minipbrt.h"
#include "raytracing.h"

class Renderer;

class Light {
public:
    virtual ~Light() = default;
    
    virtual float Illuminate(const HitInfo& hit, Renderer& renderer) = 0;
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

    float Illuminate(const HitInfo& hit, Renderer& renderer) override;
};

class AreaLight : public Light {
public:
    AreaLight(minipbrt::AreaLight* pbrtLight);
    ~AreaLight() = default;

    float Illuminate(const HitInfo& hit, Renderer& renderer) override;
};