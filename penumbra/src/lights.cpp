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
    glm::vec3 wo = glm::normalize(position - hit.p);
    sample.n = wo;
    sample.L = GetRadiance(hit);
    sample.pdf = 1.0f / (4.0f * M_PI);
    return sample;
}

LightSample DiffuseAreaLight::Sample(const HitInfo& hit, Renderer& renderer, Sampler& sampler, const Shape& shape) {
    LightSample sample;
    glm::vec3 n = hit.n;
    glm::vec3 p = shape.GetPosition();
    glm::vec3 wi = glm::normalize(p - hit.p);
    glm::vec3 wo = -wi;

    const Sphere* sphere = dynamic_cast<const Sphere*>(&shape);
    if (sphere) {

        float r = sphere->GetRadius();
        float r2 = r * r;
        glm::vec3 tl = p - hit.p;
        float d = glm::length(tl);
        float d2 = d * d;

        // Sample only visible cap directions
        float cosThetaMax = glm::sqrt(1.0f - (r2 / d2));
        float u = sampler.Sample1D();
        float cosTheta = 1.0f - u * (1.0f - cosThetaMax);
        float sinTheta = sqrt(1.0f - cosTheta * cosTheta);
        float phi = 2 * M_PI * sampler.Sample1D();
        glm::vec3 dLocal = glm::vec3(cos(phi) * sinTheta, sin(phi) * sinTheta, cosTheta);

        // Create orthonormal basis for hitpoint. TODO: Utility function for this
        glm::vec3 N = wi; 
        glm::vec3 T, B;
        if (fabs(N.x) > fabs(N.z)) {
            T = glm::normalize(glm::vec3(-N.y, N.x, 0.0f));
        } else {
            T = glm::normalize(glm::vec3(0.0f, -N.z, N.y));
        }
        B = glm::cross(N, T);

        // Find point and normal in sphere using sampled direction
        glm::vec3 dWorld = dLocal.x * T + dLocal.y * B + dLocal.z * N;
        Ray lightRay(hit.p + OCCLUDED_EPS * hit.n, dWorld);
        HitInfo lightHit;
        renderer.TraceRay(lightRay, lightHit);
        if(lightHit.shape != sphere){
            sample.p = lightHit.p;
            sample.n = normalize(sample.p - p);
            sample.L = glm::vec3(0.0f);
            sample.pdf = 0.0f;
            return sample;
        }
        sample.p = lightHit.p;
        sample.n = normalize(sample.p - p);

        // Compute PDF: 1 / visible hemisphere cap
        float cos = glm::sqrt(1.0f - (r2 / d2));
        sample.pdf = 1.0f / (2.0f * M_PI * (1.0f - cos));

        sample.L = GetRadiance(hit, shape); 
        return sample;
    }

    // TODO: Implement sampling for other shape types
    sample.p = p;
    sample.n = wo;
    sample.L = radiance;
    sample.pdf = 1.0f;
    return sample;
}

// === PDFs ===
float PointLight::Pdf(const HitInfo& hit, const glm::vec3& wo) const {
    return 0.0f;
}

float DiffuseAreaLight::Pdf(const HitInfo& hit, const Renderer& renderer, const glm::vec3& wi) const {
    const Sphere* sphere = dynamic_cast<const Sphere*>(shape);
    if (sphere) {

        // Test if ray hits and we hit light at path direction 
        HitInfo lightHit;
        Ray lightRay(hit.p + OCCLUDED_EPS * hit.n, wi);
        if (!renderer.TraceRay(lightRay, lightHit)) return 0.0f;
        if (lightHit.shape != sphere) return 0.0f;

        glm::vec3 p = sphere->GetPosition();
        float r = sphere->GetRadius();
        float r2 = r * r;
        float d = glm::length(p - hit.p);
        float d2 = d * d;

        // Compute PDF: 1 / visible hemisphere cap
        float cos = glm::sqrt(1.0f - (r2 / d2));
        return 1.0f / (2.0f * M_PI * (1.0f - cos));
    }
    // TODO: Implement PDF for other shape types
    return 0.0f;
}
