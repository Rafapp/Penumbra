#include "shading.h"
#include "lights.h"
#include <iostream>

glm::vec3 Shading::ShadeMaterial(const HitInfo& hit, Material* material, const std::vector<Light*>& lights) {
    if (!material) return glm::vec3(0.0f);
    
    if (material->GetType() == minipbrt::MaterialType::Matte) {
        return ShadeMatte(hit, static_cast<MatteMaterial*>(material), lights);
    }
    
    return glm::vec3(0.0f);
}

glm::vec3 Shading::ShadeMatte(const HitInfo& hit, MatteMaterial* matte, const std::vector<Light*>& lights) {
    glm::vec3 albedo = matte->GetAlbedo();
    glm::vec3 color = glm::vec3(0.0f);
    
    for (Light* light : lights) {
        glm::vec3 toLight = glm::normalize(light->GetPosition() - hit.p);
        float cosTheta = glm::max(0.0f, glm::dot(hit.n, toLight));
        
        // Lambertian diffuse BRDF
        float ambient = 0.5f;
        color += ambient * albedo + albedo * light->GetIntensity() * cosTheta / float(M_PI);
    }
    
    return color;
}