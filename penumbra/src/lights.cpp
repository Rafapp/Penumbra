#include <iostream>

#include "lights.h"
#include "pbrtconverter.h"
#include "renderer.h"

#define SHADOW_EPS 1e-4f

// === Constructors ===
PointLight::PointLight(minipbrt::PointLight* pbrtLight) {
    if (!pbrtLight) return;
    position = glm::vec3(pbrtLight->from[0], pbrtLight->from[1], pbrtLight->from[2]);  // Translation component
    this->intensity= glm::vec3(pbrtLight->I[0], pbrtLight->I[1], pbrtLight->I[2]);
}

AreaLight::AreaLight(minipbrt::AreaLight* pbrtAreaLight) {
    if (!pbrtAreaLight) return;
    this->type = AreaLightType::Diffuse;
    this->scale = glm::vec3(pbrtAreaLight->scale[0], pbrtAreaLight->scale[1], pbrtAreaLight->scale[2]);
}

DiffuseAreaLight::DiffuseAreaLight(minipbrt::DiffuseAreaLight* pbrtDiffuseAreaLight) : AreaLight(pbrtDiffuseAreaLight){
    if (!pbrtDiffuseAreaLight) return;
    this->intensity= glm::vec3(pbrtDiffuseAreaLight->L[0], pbrtDiffuseAreaLight->L[1], pbrtDiffuseAreaLight->L[2]);
    this->twoSided = pbrtDiffuseAreaLight->twosided;
    this->samples = pbrtDiffuseAreaLight->samples;
}

// === Shadow Factor ===
glm::vec3 PointLight::Illuminated(const HitInfo& hit, const Renderer& renderer) {
    glm::vec3 toLight = position - hit.p;
    float dist = glm::length(toLight);
    Ray shadowRay(hit.p + hit.n * SHADOW_EPS, glm::normalize(toLight));
    return renderer.TraceShadowRay(shadowRay, dist) * intensity;
}

glm::vec3 AreaLight::Illuminated(const HitInfo& hit, const Renderer& renderer, const Shape& shape) {
    if(this->GetType() == AreaLightType::Diffuse) {
        auto diffuseAreaLight = static_cast<DiffuseAreaLight*>(this);
        return diffuseAreaLight->Illuminated(hit, renderer, shape);
    }
    // TODO: More area light types
}

glm::vec3 DiffuseAreaLight::Illuminated(const HitInfo& hit, const Renderer& renderer, const Shape& shape) {
    glm::vec3 toLight = shape.GetPosition() - hit.p;
    float dist = glm::length(toLight);
    Ray shadowRay(hit.p + hit.n * SHADOW_EPS, glm::normalize(toLight));
    return renderer.TraceShadowRay(shadowRay, dist) * intensity;
}

// === Radiance ===
glm::vec3 IdealLight::GetRadiance(const HitInfo& hit) {
    return intensity;
}

glm::vec3 PointLight::GetRadiance(const HitInfo& hit) {
    float toLight = glm::length(position - hit.p);
    return intensity / (toLight * toLight);
}

glm::vec3 DiffuseAreaLight::GetRadiance(const HitInfo& hit) {
    // TODO: Implement
    // float toLight = glm::length(getP- hit.p);
    // return intensity / (toLight * toLight);
    return glm::vec3(0.0f);
}

// === Sampling === 
LightSample PointLight::Sample(const HitInfo& hit, const Renderer& renderer) {
    LightSample sample;
    sample.position = position;
    sample.normal = hit.n;
    sample.radiance = GetRadiance(hit);
    sample.pdf = 1.0f / (4.0f * M_PI);
    return sample;
}

LightSample DiffuseAreaLight::Sample(const HitInfo& hit, const Renderer& renderer) {
    LightSample sample;
    sample.position = hit.p;
    sample.normal = hit.n;
    sample.radiance = GetRadiance(hit);
    sample.pdf = 1.0f / GetSurfaceArea(); // TODO: Correct with rejection sampling etc.
}