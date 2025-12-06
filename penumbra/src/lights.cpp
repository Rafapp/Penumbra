#include <iostream>

#include "lights.h"
#include "pbrtconverter.h"
#include "renderer.h"

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
    return renderer.TraceShadowRay(hit.p, glm::normalize(toLight), hit.n, dist) * intensity;
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
    return renderer.TraceShadowRay(hit.p, glm::normalize(toLight), hit.n, dist) * intensity;
}

// === Radiance ===
glm::vec3 IdealLight::GetRadiance(const HitInfo& hit) {
    return intensity;
}

glm::vec3 PointLight::GetRadiance(const HitInfo& hit) {
    float toLight = glm::length(position - hit.p);
    return intensity / (toLight * toLight);
}

// TODO: Remove shape arg?
glm::vec3 DiffuseAreaLight::GetRadiance(const HitInfo& hit, const Shape& shape) {
   return intensity; 
}

// === Sampling === 
LightSample PointLight::Sample(const HitInfo& hit, Sampler& sampler) {
    LightSample sample;
    sample.position = position;
    sample.normal = hit.n;
    sample.radiance = GetRadiance(hit);
    sample.pdf = 1.0f / (4.0f * M_PI);
    return sample;
}

LightSample DiffuseAreaLight::Sample(const HitInfo& hit, Sampler& sampler, const Shape& shape) {
    LightSample sample;

    const Sphere* sphere = dynamic_cast<const Sphere*>(&shape);
    if (sphere) {
        glm::vec3 center = sphere->GetPosition();
        float radius = sphere->GetRadius();
        glm::vec3 dir = sampler.SampleSphereUniform();
        glm::vec3 lightPoint = center + radius * dir;
        sample.position = lightPoint;
        sample.normal = dir;
        sample.radiance = intensity;
        sample.pdf = 1.0f / (4.0f * M_PI * radius * radius);
        return sample;
    }

    //  TODO: Implement other shape types
    sample.position = shape.GetPosition();
    sample.normal   = hit.n;
    sample.radiance = intensity;
    sample.pdf      = 1.0f;
    return sample;
}

// === PDFs ===
float PointLight::Pdf(const HitInfo& hit, const glm::vec3& wo) const {
    return 0.0f;
}

float DiffuseAreaLight::Pdf(const HitInfo& hit, const Renderer& renderer, const Shape& shape, const glm::vec3& wo) const {
    const Sphere* sphere = dynamic_cast<const Sphere*>(&shape);
    if (sphere) {
        Ray r(hit.p, wo);
        HitInfo lightHit;
        if (!renderer.IntersectRayScene(r,lightHit)) return 0.0f;

        float dist2 = lightHit.t * lightHit.t;
        glm::vec3 n = lightHit.n;
        float cosLight = glm::dot(-wo, n);
        if (cosLight <= 0.0f) return 0.0f;

        if (renderer.TraceShadowRay(hit.p, wo, hit.n, lightHit.t) == 0.0f) return 0.0f;

        return (1.0f / surfaceArea) * dist2 / cosLight;
    }
    return 0.0f;
}
