#include <iostream>

#include "shading.h"
#include "lights.h"


// === Utilities ===
inline float ShlickFresnel(float cos, float eta) {
    float Fo = (eta - 1.0f) * (eta - 1.0f) / ((eta + 1.0f) * (eta + 1.0f));
    return Fo + (1.0f - Fo) * glm::pow(1.0f - cos, 5.0f);
}

inline float GGX(float r, glm::vec3 h, glm::vec3 v) {
    float r2 = r * r;
    float cos = glm::dot(h, v);
    float base = glm::pow(cos, 2) * (r2 - 1.0f) + 1.0f;
    float D = r2 / (M_PI * glm::pow(base, 2));
    return D;
}


// === BxDF Sampling ===    
Shading::BxDFSample Shading::SampleMaterial(const HitInfo& hit, 
                                            const glm::vec3& wi, 
                                            const glm::vec3& wo, 
                                            const Material* material, 
                                            Sampler& sampler) {

    if (material->GetType() == minipbrt::MaterialType::Matte) {
        auto matte = static_cast<const MatteMaterial*>(material);
        return SampleMatte(hit, wo, wi, matte, sampler);
    }
	else if (material->GetType() == minipbrt::MaterialType::Disney) {
        auto disney = static_cast<const DisneyMaterial*>(material);
        return SampleDisney(hit, wi, wo, disney, sampler);
    }
    // Add other material types
    return {};
}

Shading::BxDFSample Shading::SampleMatte(const HitInfo& hit, 
                                         const glm::vec3& wi, 
                                         const glm::vec3& wo, 
                                         const MatteMaterial* matte, 
                                         Sampler& sampler){

    BxDFSample sample;
    sample.wo = sampler.SampleHemisphereCosine(hit.n);
    sample.color = matte->albedo / M_PI; // Lambertian BRDF
    sample.pdf = glm::max(0.0f, glm::dot(hit.n, sample.wo)) / M_PI;
    return sample;
}

// "Physically-Based Shading at Disney," Brent Burley (2012)
// "Extending the Disney BRDF to a BSDF with Integrated Subsurface Scattering," Brent Burley (2015)
Shading::BxDFSample Shading::SampleDisney(const HitInfo& hit, 
                                          const glm::vec3& wi, 
                                          const glm::vec3& wo, 
                                          const DisneyMaterial* disney, 
                                          Sampler& sampler) {
    BxDFSample sample;

    // Variables
    float r = disney->roughness;
    float r2 = r * r;
	glm::vec3 n = hit.front ? hit.n : -hit.n;
    glm::vec3 h = sampler.SampleHemisphereGGX(n, r);
	float hDotWi = glm::max(0.0f, glm::dot(h, wi)); // TODO: Need wi?
	float hDotWo = glm::max(0.0f, glm::dot(h, wo));

    // Ignore backface reflections
	glm::vec3 dReflect = glm::reflect(wo, h);
    if (glm::dot(dReflect, n) <= 0.0f) {
        sample.pdf = 0.0f;
        return sample;  
    }

    // Metallic: Reflection 
    if (disney->metallic >= 0.99f) {
        // Reflection probability 
        float pGGX = GGX(r, h, n);
        float pReflect = pGGX * hDotWi / (4.0f * hDotWo);

        // Reflection weight
        glm::vec3 fBrdf = ShadeDisney(hit, wo, dReflect, disney);
        glm::vec3 color = fBrdf / pReflect;

        sample.wo = dReflect;
        sample.color = color;
        sample.pdf = pReflect;
        return sample;
    }

    // Dielectric: Refraction
    else {

        // Variables
        float eta = hit.front ? 1.0f / disney->eta : disney->eta;
        float F = ShlickFresnel(hDotWo, eta);
	    float nDotH = glm::max(0.0f, glm::dot(n, h));
        float nDotWo = glm::max(0.0f, glm::dot(n, wo));

        // Decide response based on Fresnel 
		float x = sampler.Sample1D();

        // I. Reflect
        if (x < F) {

            // Reflection probability 
            float GGXreflect = GGX(r, h, n);
            float pReflect = GGXreflect * hDotWi / (4.0f * hDotWo);

            // Reflection weight
            glm::vec3 fBrdf = ShadeDisney(hit, wo, dReflect, disney);
            glm::vec3 color  = fBrdf * F / pReflect;

            // Return sample
            sample.color = color;
            sample.pdf = pReflect;
            sample.wo = dReflect;
			return sample;
        }

        // II. Refract
        else {
            glm::vec3 dRefract = glm::refract(wo, h, eta);

            // Case 1: Total internal reflection
            if (glm::length(dRefract) == 0.0f) {

                // Reflection probability 
                float GGXreflect = GGX(r, h, n);
                float pReflect = GGXreflect * hDotWi / (4.0f * hDotWo * eta * eta);

                // Reflection weight
                glm::vec3 fBrdf = ShadeDisney(hit, wo, dReflect, disney);
                glm::vec3 color = fBrdf * F / pReflect;

                glm::vec3 dReflect = glm::reflect(wo, n);
                sample.color = color;
                sample.pdf = pReflect;
                sample.wo = dReflect;
                return sample;
            }

            // Case 2: Refraction
            else {

                // Refraction probability 
                float GGXrefract = GGX(r, h, n);
                float pRefract = GGXrefract * hDotWi / (eta * eta * glm::dot(dRefract, wo));

                // Refraction weight
                glm::vec3 fBssrdf = ShadeDisney(hit, wo, dRefract, disney);
				glm::vec3 color = fBssrdf * (1.0f - F) / pRefract;

                // Return sample
                sample.color = color;
				sample.wo = dRefract;
                sample.pdf = pRefract;
                return sample;
			}
        }
    }

    return sample;
}

// === BxDF Shading ===
glm::vec3 Shading::ShadeMaterial(const HitInfo& hit, 
                                 const glm::vec3& wi, 
                                 const glm::vec3& wo, 
                                 const Material* material) {

    if (material->GetType() == minipbrt::MaterialType::Matte) {
        return ShadeMatte(hit, wo, wi, static_cast<const MatteMaterial*>(material));
    } else if (material->GetType() == minipbrt::MaterialType::Disney) {
        return ShadeDisney(hit, wo, wi, static_cast<const DisneyMaterial*>(material));
	}
    
    return glm::vec3(0.0f);
}

glm::vec3 Shading::ShadeMatte(const HitInfo& hit, 
                              const glm::vec3& wi, 
                              const glm::vec3& wo, 
                              const MatteMaterial* matte) {

    return glm::max(0.0f, glm::dot(hit.n, wi)) * matte->albedo / M_PI;
}

// "Physically-Based Shading at Disney," Brent Burley (2012)
// "Extending the Disney BRDF to a BSDF with Integrated Subsurface Scattering," Brent Burley (2015)
// TODO: Sampler may not be needed
glm::vec3 Shading::ShadeDisney(const HitInfo& hit, 
                               const glm::vec3& wi, 
                               const glm::vec3& wo, 
                               const DisneyMaterial* disney) {

    // Variables
	glm::vec3 h = glm::normalize(wi + wo);
	float r = disney->roughness;
    float nDotWi = glm::max(0.0f, glm::dot(hit.n, wi));
    float nDotWo = glm::max(0.0f, glm::dot(hit.n, wo));
    if (nDotWi <= 0.0f || nDotWo <= 0.0f) return glm::vec3(0.0f); // Backface
    float hDotWi = glm::max(0.0f, glm::dot(h, wi));
    float hDotWo = glm::max(0.0f, glm::dot(h, wo));

    // Dielectric BRDF with integrated subsurface scattering
    float hCos2 = hDotWi * hDotWi;
    float fd90 = 0.5f + 2.0f * disney->roughness * hCos2;
    float fd = (1.0f + (fd90 - 1.0f) * (glm::pow((1.0f - nDotWi), 5))) * 
               (1.0f + (fd90 - 1.0f) * (glm::pow((1.0f - nDotWo), 5)));
    glm::vec3 diffuse = (disney->albedo / M_PI) * (1.0f - disney->metallic) * fd;

    // Specular BRDF
	glm::vec3 Fo = glm::mix(glm::vec3(0.04f), disney->albedo, disney->metallic);
    glm::vec3 F = Fo + (1.0f - Fo) * glm::pow(1.0f - hDotWo, 5.0f);
    float K = glm::pow((r + 1.0f),2) / 8.0f;
	float Gwi = nDotWi / (nDotWi * (1.0f - K) + K);
	float Gwo = nDotWo / (nDotWo * (1.0f - K) + K);
    float G = Gwi * Gwo;
	float D = GGX(r, h, hit.n);
	glm::vec3 specular = (F * G * D) / (4.0f * nDotWi * nDotWo);

    return diffuse + specular;
}

// === BxDF PDF's ===
float Shading::PdfMaterial(const HitInfo& hit, 
                           const glm::vec3& wi, 
                           const glm::vec3& wo,
                           const Material* mat,
                           Sampler& sampler) {

    if (mat->GetType() == minipbrt::MaterialType::Matte) {
        return PdfMatte(hit, wi, wo, static_cast<const MatteMaterial*>(mat), sampler);
    }
    else if (mat->GetType() == minipbrt::MaterialType::Disney) {
        return PdfDisney(hit, wi, wo, static_cast<const DisneyMaterial*>(mat), sampler);
    }
    return 0.0f;
}

float Shading::PdfMatte(const HitInfo& hit, 
                        const glm::vec3& wi,
                        const glm::vec3& wo, 
                        const MatteMaterial* matte,
                        Sampler& sampler) {

    float cosTheta = glm::max(0.0f, glm::dot(hit.n, wi));
    return cosTheta * M_1_PI;
}

float Shading::PdfDisney(const HitInfo& hit, 
                         const glm::vec3& wi,
                         const glm::vec3& wo, 
                         const DisneyMaterial* disney,
                         Sampler& sampler) {
	float r = disney->roughness;
	//glm::vec3 h = sampler.SampleHemisphereGGX(hit.n, r);
	glm::vec3 h = glm::normalize(wi + wo);
    float D = GGX(r, h, hit.n);
	float hDotWi = glm::max(0.0f, glm::dot(h, wi));
	float hDotWo = glm::max(0.0f, glm::dot(h, wo));
    return D * hDotWi / (4.0f * hDotWo);
}