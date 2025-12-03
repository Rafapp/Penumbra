#include <iostream>

#include "shading.h"
#include "lights.h"

    
Shading::BxDFSample Shading::SampleMaterial(const HitInfo& hit, Material* material, const glm::vec3& wi, Sampler& sampler) {
    if (material->GetType() == minipbrt::MaterialType::Matte) {
        auto matte = static_cast<MatteMaterial*>(material);
        return SampleMatte(hit, matte, wi, sampler);
    }
    // Add other material types
    return {};
}

Shading::BxDFSample Shading::SampleMatte(const HitInfo& hit, MatteMaterial* matte, const glm::vec3& wi, Sampler& sampler){
    BxDFSample sample;
    sample.direction = sampler.SampleHemisphereCosine(hit.n);
    sample.color = matte->GetAlbedo() / float(M_PI); // Lambertian BRDF
    sample.pdf = glm::max(0.0f, glm::dot(hit.n, sample.direction)) / float(M_PI);
    return sample;
}

glm::vec3 Shading::ShadeMaterial(const HitInfo& hit, Material* material, const std::vector<Light*>& lights) {
    if (!material) return glm::vec3(0.0f);
    
    if (material->GetType() == minipbrt::MaterialType::Matte) {
        return ShadeMatte(hit, static_cast<MatteMaterial*>(material), lights);
    }
    
    return glm::vec3(0.0f);
}

// glm::vec3 Shading::ShadeMatte(const HitInfo& hit, MatteMaterial* matte, const std::vector<Light*>& lights) {
//     glm::vec3 albedo = matte->GetAlbedo();
//     glm::vec3 color = glm::vec3(0.0f);
    
//     for (Light* light : lights) {
//         glm::vec3 toLight = glm::normalize(light->GetPosition() - hit.p);
//         float cosTheta = glm::max(0.0f, glm::dot(hit.n, toLight));
        
//         // Lambertian diffuse BRDF
//         float ambient = 0.5f;
//         color += albedo * cosTheta / float(M_PI);
//     }
    
//     return color;
// }