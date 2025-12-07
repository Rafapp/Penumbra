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
    if(pbrtAreaLight->type() == minipbrt::AreaLightType::Diffuse) {
        this->type = AreaLightType::Diffuse;
        this->scale = glm::vec3(pbrtAreaLight->scale[0], pbrtAreaLight->scale[1], pbrtAreaLight->scale[2]);
    }
    // TODO: More area light types
}

DiffuseAreaLight::DiffuseAreaLight(minipbrt::DiffuseAreaLight* pbrtDiffuseAreaLight) : AreaLight(pbrtDiffuseAreaLight){
    if (!pbrtDiffuseAreaLight) return;
    this->radiance = glm::vec3(pbrtDiffuseAreaLight->L[0], pbrtDiffuseAreaLight->L[1], pbrtDiffuseAreaLight->L[2]);
    this->twoSided = pbrtDiffuseAreaLight->twosided;
    this->samples = pbrtDiffuseAreaLight->samples;
}

// === Shadow Factor ===
bool PointLight::Visible(const HitInfo& hit, const Renderer& renderer) {
    glm::vec3 toLight = position - hit.p;
    float dist = glm::length(toLight);
    return renderer.Occluded(hit.p, glm::normalize(toLight), hit.n, dist);
}

bool AreaLight::Visible(const HitInfo& hit, const Renderer& renderer) {
    if(this->GetType() == AreaLightType::Diffuse) {
        auto diffuseAreaLight = static_cast<DiffuseAreaLight*>(this);
        return diffuseAreaLight->Visible(hit, renderer);
    }
    // TODO: More area light types
    return false;
}

bool DiffuseAreaLight::Visible(const HitInfo& hit, const Renderer& renderer) {
    if (!this->shape) return false;
    glm::vec3 toLight = this->shape->GetPosition() - hit.p;
    float dist = glm::length(toLight);
    return renderer.Occluded(hit.p, glm::normalize(toLight), hit.n, dist);
}

// === Radiance ===
glm::vec3 IdealLight::GetRadiance(const HitInfo& hit) {
    return intensity;
}

glm::vec3 PointLight::GetRadiance(const HitInfo& hit) {
    float d = glm::length(position - hit.p);
    return intensity / (d * d);
}

// === Sampling === 
LightSample PointLight::Sample(const HitInfo& hit, Sampler& sampler) {
    LightSample sample;
    sample.p = position;
    sample.n = hit.n;
    sample.L = GetRadiance(hit);
    sample.pdf = 1.0f / (4.0f * M_PI);
    return sample;
}

LightSample DiffuseAreaLight::Sample(const HitInfo& hit, Sampler& sampler, const Shape& shape) {
    glm::vec3 p = shape.GetPosition();
    glm::vec3 n = hit.n;

    // Fallback
    LightSample sample;
    sample.p = p;
    sample.n = n;
    sample.L = radiance;
    sample.pdf = 1.0f;

    // Sample point in sphere equator disk
    const Sphere* sphere = dynamic_cast<const Sphere*>(&shape);
    if (sphere) {

        glm::vec3 p = shape.GetPosition();
        glm::vec3 toLight = p - hit.p;
        glm::vec3 wi = glm::normalize(toLight);
        glm::vec3 wo = -wi;

        // Distance to equator disk
        float d = glm::length(toLight);
        float d2 = d * d;
        float r = sphere->GetRadius();
        float r2 = r * r;
        float t = (d2 - r2) / d;

        // Sample disk and scale to radius
        glm::vec2 sdu = sampler.SampleUnitDiskUniform();
        glm::vec2 sdv = sdu * r;

        // Orthonormal basis for disk
        glm::vec3 b1 = glm::normalize(glm::cross(wo, glm::vec3(0, 1, 0)));
        glm::vec3 b2 = glm::cross(wo, b1);

        // Find position and normal of point on disk 
        glm::vec3 pd = (hit.p + t * wi) + (sdv.x * b1) + (sdv.y * b2);

        sample.p = pd;
        sample.n = wo;

        // Compute PDF: 1 / disk area
        float da = M_PI * r2;
        sample.pdf = 1.0f / da;

        // Note: GetRadiance() scales by 4pi to convert radiance to total emitted power
        sample.L = GetRadiance(hit, shape); 
        return sample;
    }

    // TODO: Implement sampling for other shape types
    return sample;
}

// === PDFs ===
float PointLight::Pdf(const HitInfo& hit, const glm::vec3& wo) const {
    return 0.0f;
}

float DiffuseAreaLight::Pdf(const HitInfo& hit, const Renderer& renderer, const glm::vec3& wo) const {
    const Sphere* sphere = dynamic_cast<const Sphere*>(shape);
    if (sphere) {
        // Find distance squared to hemisphere disk
        glm::vec3 p = shape->GetPosition();
        float d = glm::length(p - hit.p);
        float d2 = d * d;
                
        // Compute PDF: We assume hit.n dot wi > 0
        float r = sphere->GetRadius();
        float r2 = r * r;
        return d2 / (M_PI * r2);
    }
    // TODO: Implement PDF for other shape types
    return 0.0f;
}
