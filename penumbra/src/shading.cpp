#include <iostream>

#include "shading.h"
#include "lights.h"

glm::vec3 Shading::SampleHemisphereUniform(const glm::vec3& normal, Sampler& sampler) {
   // Sample hemisphere uniformly around the normal
   float u1 = sampler.Sample1D();
   float u2 = sampler.Sample1D();

   float r = sqrt(u1);
   float theta = 2.0f * M_PI * u2;

   float x = r * cos(theta);
   float y = r * sin(theta);
   float z = sqrt(glm::max(0.0f, 1.0f - u1));

   // Create an orthonormal basis (T, B, N)
   glm::vec3 N = glm::normalize(normal);
   glm::vec3 T, B;
   if (fabs(N.x) > fabs(N.z)) {
       T = glm::normalize(glm::vec3(-N.y, N.x, 0.0f));
   } else {
       T = glm::normalize(glm::vec3(0.0f, -N.z, N.y));
   }
   B = glm::cross(N, T);

   // Transform sample to world space
   glm::vec3 sampleDir = x * T + y * B + z * N;
   return glm::normalize(sampleDir);
}

glm::vec3 Shading::SampleHemisphereCosine(const glm::vec3& normal, Sampler& sampler) {
    // Sample hemisphere with cosine-weighted distribution around the normal
    float u1 = sampler.Sample1D();
    float u2 = sampler.Sample1D();

    float r = sqrt(u1);
    float theta = 2.0f * M_PI * u2;

    float x = r * cos(theta);
    float y = r * sin(theta);
    float z = sqrt(glm::max(0.0f, 1.0f - u1));

    // Create an orthonormal basis (T, B, N)
    glm::vec3 N = glm::normalize(normal);
    glm::vec3 T, B;
    if (fabs(N.x) > fabs(N.z)) {
        T = glm::normalize(glm::vec3(-N.y, N.x, 0.0f));
    } else {
        T = glm::normalize(glm::vec3(0.0f, -N.z, N.y));
    }
    B = glm::cross(N, T);

    // Transform sample to world space
    glm::vec3 sampleDir = x * T + y * B + z * N;
    return glm::normalize(sampleDir);
}
    
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
    sample.direction = SampleHemisphereCosine(hit.n, sampler);
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

glm::vec3 Shading::ShadeMatte(const HitInfo& hit, MatteMaterial* matte, const std::vector<Light*>& lights) {
    glm::vec3 albedo = matte->GetAlbedo();
    glm::vec3 color = glm::vec3(0.0f);
    
    for (Light* light : lights) {
        glm::vec3 toLight = glm::normalize(light->GetPosition() - hit.p);
        float cosTheta = glm::max(0.0f, glm::dot(hit.n, toLight));
        
        // Lambertian diffuse BRDF
        float ambient = 0.5f;
        color += albedo * cosTheta / float(M_PI);
    }
    
    return color;
}