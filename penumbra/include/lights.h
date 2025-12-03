#pragma once

#include "glm/glm.hpp"
#include "minipbrt.h"
#include "raytracing.h"
#include "shapes.h"

class Renderer;

// TODO: Polymorphism might not be ideal
class Light{};

// === Ideal lights ===
class IdealLight : public Light {
public:
    virtual ~IdealLight() = default;
    
    virtual glm::vec3 Illuminated(const HitInfo& hit, Renderer& renderer) = 0;
    virtual glm::vec3 GetRadiance(HitInfo& hit);
    glm::vec3 GetPosition() const { return position; }

protected:
    glm::vec3 position;
    glm::vec3 intensity;
};

class PointLight : public IdealLight {
public:
    PointLight(minipbrt::PointLight* pbrtLight);
    ~PointLight() = default;

    glm::vec3 Illuminated(const HitInfo& hit, Renderer& renderer) override;
    glm::vec3 GetRadiance(HitInfo& hit) override;
};

// === Area lights ===

enum class AreaLightType {
    Diffuse,
    // TODO: Add more area light types
};

class AreaLight : public Light {
public:
    AreaLight(minipbrt::AreaLight* pbrtAreaLight);
    ~AreaLight() = default;

    AreaLightType GetType() const { return type; }

    virtual glm::vec3 Illuminated(const HitInfo& hit, Renderer& renderer, Shape& shape) = 0;
private:
    AreaLightType type;
    glm::vec3 scale;
};

class DiffuseAreaLight : public AreaLight {
public:
    DiffuseAreaLight(minipbrt::DiffuseAreaLight* pbrtAreaLight);
    ~DiffuseAreaLight() = default;

    glm::vec3 Illuminated(const HitInfo& hit, Renderer& renderer, Shape& shape) override;
    float GetSurfaceArea() const { return surfaceArea; }
private:
    glm::vec3 intensity;
    bool twoSided;
    int samples;
    float surfaceArea;
};

namespace Lights{
    struct LightSample{
        glm::vec3 position;
        glm::vec3 normal;
        glm::vec3 radiance;
        float pdf;
    };
    LightSample SampleIdealLight(IdealLight* light, HitInfo& hit, Renderer& renderer);
    LightSample SampleAreaLight(AreaLight* areaLight, HitInfo& hit, Renderer& renderer);
    LightSample SampleDiffuseAreaLight(DiffuseAreaLight* diffuseAreaLight, HitInfo& hit, Renderer& renderer);

};