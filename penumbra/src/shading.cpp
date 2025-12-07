#include <iostream>

#include "shading.h"
#include "lights.h"

// === BxDF Sampling ===    
Shading::BxDFSample Shading::SampleMaterial(const HitInfo& hit, const Material* material, const glm::vec3& wi, Sampler& sampler) {
    if (material->GetType() == minipbrt::MaterialType::Matte) {
        auto matte = static_cast<const MatteMaterial*>(material);
        return SampleMatte(hit, matte, wi, sampler);
    }
    // Add other material types
    return {};
}

Shading::BxDFSample Shading::SampleMatte(const HitInfo& hit, const MatteMaterial* matte, const glm::vec3& wi, Sampler& sampler){
    BxDFSample sample;
    sample.d = sampler.SampleHemisphereCosine(hit.n);
    sample.color = matte->GetAlbedo() / float(M_PI); // Lambertian BRDF
    sample.pdf = glm::max(0.0f, glm::dot(hit.n, sample.d)) / float(M_PI);
    return sample;
}

// === BxDF Shading ===
glm::vec3 Shading::ShadeMaterial(const HitInfo& hit, const glm::vec3& wi, const Material* material) {
    if (!material) return glm::vec3(0.0f);
    
    if (material->GetType() == minipbrt::MaterialType::Matte) {
        return ShadeMatte(hit, wi, static_cast<const MatteMaterial*>(material));
    }
    
    return glm::vec3(0.0f);
}

glm::vec3 Shading::ShadeMatte(const HitInfo& hit, const glm::vec3& wi, const MatteMaterial* matte) {
    return matte->GetAlbedo() / float(M_PI);
}

// === BxDF PDF's ===
float Shading::PdfMaterial(const HitInfo& hit, const glm::vec3& wi, const Material* mat) {
    if (mat->GetType() == minipbrt::MaterialType::Matte) {
        const MatteMaterial* matte = static_cast<const MatteMaterial*>(mat);
        return PdfMatte(hit, matte, wi);
    }
    return 0.0f;
}

float Shading::PdfMatte(const HitInfo& hit, const MatteMaterial* matte, const glm::vec3& wi) {
    float cosTheta = glm::max(0.0f, glm::dot(hit.n, wi));
    return cosTheta / float(M_PI);
}