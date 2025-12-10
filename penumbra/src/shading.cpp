#include <iostream>

#include "shading.h"
#include "lights.h"

// === BxDF Sampling ===    
Shading::BxDFSample Shading::SampleMaterial(const HitInfo& hit, const Material* material, const glm::vec3& d, Sampler& sampler) {
    if (material->GetType() == minipbrt::MaterialType::Matte) {
        auto matte = static_cast<const MatteMaterial*>(material);
        return SampleMatte(hit, matte, d, sampler);
    }
	else if (material->GetType() == minipbrt::MaterialType::Disney) {
        auto disney = static_cast<const DisneyMaterial*>(material);
        return SampleDisney(hit, disney, d, sampler);
    }
    // Add other material types
    return {};
}

Shading::BxDFSample Shading::SampleMatte(const HitInfo& hit, const MatteMaterial* matte, const glm::vec3& d, Sampler& sampler){
    BxDFSample sample;
    sample.d = sampler.SampleHemisphereCosine(hit.n);
    sample.color = matte->albedo / M_PI; // Lambertian BRDF
    sample.pdf = glm::max(0.0f, glm::dot(hit.n, sample.d)) / M_PI;
    return sample;
}

Shading::BxDFSample Shading::SampleDisney(const HitInfo& hit, const DisneyMaterial* disney, const glm::vec3& d, Sampler& sampler) {
    BxDFSample sample;
    return sample;
}

// === BxDF Shading ===
glm::vec3 Shading::ShadeMaterial(const HitInfo& hit, const glm::vec3& wo, const glm::vec3& wi, const Material* material) {
    if (!material) return glm::vec3(0.0f);
    
    if (material->GetType() == minipbrt::MaterialType::Matte) {
        return ShadeMatte(hit, wo, wi, static_cast<const MatteMaterial*>(material));
    } else if (material->GetType() == minipbrt::MaterialType::Disney) {
        return ShadeDisney(hit, wo, wi, static_cast<const DisneyMaterial*>(material));
	}
    
    return glm::vec3(0.0f);
}

glm::vec3 Shading::ShadeMatte(const HitInfo& hit, const glm::vec3& wo, const glm::vec3& wi, const MatteMaterial* matte) {
    return glm::max(0.0f, glm::dot(hit.n, wi)) * matte->albedo / M_PI;
}

inline float ShlickFresnel(float cos, float eta, bool front) {
	if (front) eta = 1.0f / eta;
	float F0 = (eta - 1.0f) * (eta - 1.0f) / ((eta + 1.0f) * (eta + 1.0f));
	return F0 + (1.0f - F0) * glm::pow(1.0f - cos, 5.0f);
}

glm::vec3 Shading::ShadeDisney(const HitInfo& hit, const glm::vec3& wo, const glm::vec3& wi, const DisneyMaterial* disney) {
	glm::vec3 h = glm::normalize(wo + wi);

    // Burley Diffuse
	float nDotWi = glm::max(0.0f, glm::dot(hit.n, wi));
	float nDotWo = glm::max(0.0f, glm::dot(hit.n, wo));
    if (nDotWi <= 0.0f || nDotWo <= 0.0f) return glm::vec3(0.0f); // Backface
    float hDotWi = glm::max(0.0f, glm::dot(h, wi));
    float hCos2 = hDotWi * hDotWi;
    float fd90 = 0.5f + 2.0f * disney->roughness * hCos2;
    float fd = (1.0f + (fd90 - 1.0f) * (glm::pow((1.0f - nDotWi), 5))) * 
               (1.0f + (fd90 - 1.0f) * (glm::pow((1.0f - nDotWo), 5)));
    glm::vec3 diffuse = (disney->albedo / M_PI) * (1.0f - disney->metallic) * fd;
    return diffuse;
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

float Shading::PdfDisney(const HitInfo& hit, const DisneyMaterial* disney, const glm::vec3& wi) {
    return 0.0f;
}