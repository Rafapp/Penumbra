#include <iostream>

#include "shading.h"
#include "lights.h"

    
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
    sample.direction = sampler.SampleHemisphereCosine(hit.n);
    sample.color = matte->GetAlbedo() / float(M_PI); // Lambertian BRDF
    sample.pdf = glm::max(0.0f, glm::dot(hit.n, sample.direction)) / float(M_PI);
    return sample;
}

glm::vec3 Shading::ShadeMaterial(const HitInfo& hit, const Material* material, const Light* light) {
    if (!material) return glm::vec3(0.0f);
    
    if (material->GetType() == minipbrt::MaterialType::Matte) {
        return ShadeMatte(hit, static_cast<const MatteMaterial*>(material), light);
    }
    
    return glm::vec3(0.0f);
}

glm::vec3 Shading::ShadeMatte(const HitInfo& hit, const MatteMaterial* matte, const Light* light) {
    glm::vec3 albedo = matte->GetAlbedo();
    glm::vec3 color = glm::vec3(0.0f);
    glm::vec3 toLight;
    // TODO: Bad polymorphism ... rethink
    if(const IdealLight* idealLight = static_cast<const IdealLight*>(light)){
        toLight = glm::normalize(idealLight->GetPosition() - hit.p);
    } else if(const AreaLight* areaLight = dynamic_cast<const AreaLight*>(light)){
        // TODO: Sample point on area light surface instead
        // toLight = glm::normalize(areaLight->GetPosition() - hit.p);
    }
    float cosTheta = glm::max(0.0f, glm::dot(hit.n, toLight));
        
    // Lambertian diffuse BRDF
    float ambient = 0.5f;
    color += albedo * cosTheta / float(M_PI);
    
    return color;
}