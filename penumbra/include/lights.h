#pragma once

#include "glm/glm.hpp"
#include "minipbrt.h"
#include "raytracing.h"
#include "shapes.h"

class Renderer;
class Sampler;

struct LightSample{
    glm::vec3 p;
    glm::vec3 n;
    glm::vec3 L;
    float pdf;
};

// TODO: Polymorphism might not be ideal
class Light{
public:
    virtual ~Light() = default;
};

// === Ideal lights ===
class IdealLight : public Light {
public:
    virtual ~IdealLight() = default;

    virtual bool Visible(const HitInfo& hit, const Renderer& renderer) = 0;
    virtual glm::vec3 GetRadiance(const HitInfo& hit) = 0;
    virtual LightSample Sample(const HitInfo& hit, Sampler& sampler) = 0;
    virtual float Pdf(const HitInfo& hit, const glm::vec3& wo) const = 0;
    glm::vec3 GetPosition() const { return position; }

protected:
    glm::vec3 position;
    glm::vec3 intensity;
};

class PointLight : public IdealLight {
public:
    PointLight(minipbrt::PointLight* pbrtLight);
    ~PointLight() = default;

    LightSample Sample(const HitInfo& hit, Sampler& sampler) override;
    bool Visible(const HitInfo& hit, const Renderer& renderer) override;
    glm::vec3 GetRadiance(const HitInfo& hit) override;
    float Pdf(const HitInfo& hit, const glm::vec3& wo) const override;
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

    virtual LightSample Sample(const HitInfo& hit, Renderer& renderer, Sampler& sampler, const Shape& shape) = 0;
    virtual bool Visible(const HitInfo& hit, const Renderer& renderer) = 0;
    virtual glm::vec3 GetRadiance(const HitInfo& hit, const Shape& shape) = 0;
    virtual float Pdf(const HitInfo& hit, const Renderer& renderer, const glm::vec3& wi) const = 0;
    Shape* shape = nullptr;
private:
    AreaLightType type;
    glm::vec3 scale;
};

class DiffuseAreaLight : public AreaLight {
public:
    DiffuseAreaLight(minipbrt::DiffuseAreaLight* pbrtAreaLight);
    ~DiffuseAreaLight() = default;

    LightSample Sample(const HitInfo& hit, Renderer& renderer, Sampler& sampler, const Shape& shape) override;
    bool Visible(const HitInfo& hit, const Renderer& renderer) override;
    glm::vec3 GetRadiance(const HitInfo& hit, const Shape& shape) override { return radiance * 4.0f * float(M_PI); }
    float GetSurfaceArea() const { return surfaceArea; }
    float Pdf(const HitInfo& hit, const Renderer& renderer, const glm::vec3& wi) const override;
private:
    glm::vec3 radiance;
    bool twoSided;
    int samples;
    float surfaceArea;
};