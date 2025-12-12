#include <iostream>

#include "shading.h"
#include "lights.h"


// === Utilities ===
inline float ShlickFresnel(float cos, float eta) {
    float Fo = (eta - 1.0f) * (eta - 1.0f) / ((eta + 1.0f) * (eta + 1.0f));
    return Fo + (1.0f - Fo) * glm::pow(1.0f - cos, 5.0f);
}

inline float DGGX(float r, glm::vec3 h, glm::vec3 v) {
    float r2 = r * r;
    float cos = glm::dot(h, v);
    float base = glm::pow(cos, 2) * (r2 - 1.0f) + 1.0f;
    float D = r2 / (M_PI * glm::pow(base, 2));
    return D;
}


// === BxDF Sampling ===    
Shading::BxDFSample Shading::SampleMaterial(const HitInfo& hit, 
                                            const glm::vec3& wi, 
                                            const Material* material, 
                                            Sampler& sampler) {

    if (material->GetType() == minipbrt::MaterialType::Matte) {
        auto matte = static_cast<const MatteMaterial*>(material);
        return SampleMatte(hit, wi, matte, sampler);
    }
	else if (material->GetType() == minipbrt::MaterialType::Disney) {
        auto disney = static_cast<const DisneyMaterial*>(material);
        return SampleDisney(hit, wi, disney, sampler);
    }
    // Add other material types
    return {};
}

Shading::BxDFSample Shading::SampleMatte(const HitInfo& hit, 
                                         const glm::vec3& wi, 
                                         const MatteMaterial* matte, 
                                         Sampler& sampler){

    BxDFSample sample;
    sample.wo = sampler.SampleHemisphereCosine(hit.n);
	float cos = glm::max(0.0f, glm::dot(hit.n, sample.wo));
    sample.pdf = cos * M_1_PI;
    sample.weight = glm::vec3(cos / sample.pdf);
    return sample;
}

// "Physically-Based Shading at Disney," Brent Burley (2012)
// "Extending the Disney BRDF to a BSDF with Integrated Subsurface Scattering," Brent Burley (2015)
Shading::BxDFSample Shading::SampleDisney(const HitInfo& hit, 
                                          const glm::vec3& wi, 
                                          const DisneyMaterial* disney, 
                                          Sampler& sampler) {
    BxDFSample sample;

    // Variables
    //float r = glm::max(0.05f, disney->roughness);
    float r = disney->roughness;
    float r2 = r * r;
	glm::vec3 n = hit.front ? hit.n : -hit.n;
	glm::vec3 h = glm::vec3(0.0f);
	float hDotWi = -1.0f;

    // Guarantee GGX sample in visible hemisphere
    // TODO: Implement Heiz VNDF
	int maxTrials = 10;
    int trials = 0;
    do {
		h = sampler.SampleHemisphereGGX(n, r);
        hDotWi = glm::dot(h, wi);
	} while (hDotWi < 0.0f && ++trials < maxTrials);

    if(hDotWi <= 0.0f) {
        sample.pdf = 0;
        return sample;
    }

	float hDotN = glm::dot(h, n);
	glm::vec3 dReflect = glm::reflect(wi, h);
	float hDotDr = glm::abs(glm::dot(h, dReflect)); // Account for reflection precision

    // A) === Pure metallic: Reflection === 
    if (disney->metallic >= 0.99f) {
        // Reflection probability 
        float D = DGGX(r, h, n);
        float pReflect = D * hDotN / (4.0f * hDotDr);

        // Reject grazing angles
		if (pReflect <= 0.0f) {
            sample.pdf = 0.0f;
            return sample;
		}

        // Reflection weight
        glm::vec3 weight = glm::vec3(1.0f / pReflect);

        sample.wo = dReflect;
        sample.weight = weight;
        sample.pdf = pReflect;
        return sample;
    }

    // B) === Dielectric: Diffuse or specular response  ===
    else {

        // --- I. DIFFUSE LOBE ---
        float pLobe = 0.5f;
        if (sampler.Sample1D() < pLobe) {
            sample.wo = sampler.SampleHemisphereCosine(n);
            float cos = glm::max(0.0f, glm::dot(n, sample.wo));
            sample.pdf = pLobe * cos / M_PI;
            glm::vec3 diffuseBrdf = (disney->albedo / M_PI) * (1.0f - disney->metallic);
            sample.weight = diffuseBrdf / sample.pdf;
            return sample;
        }

		// --- II. SPECULAR LOBE ---

        // Variables
        float eta = hit.front ? 1.0f / disney->eta : disney->eta;
		float eta2 = eta * eta;
        float F = ShlickFresnel(hDotWi, eta);
	    float nDotH = glm::max(0.0f, glm::dot(n, h));
        float nDotWo = glm::max(0.0f, glm::dot(n, wi));

        // Decide response based on Fresnel 
		float x = sampler.Sample1D();

        // 1. Reflect
        if (x < F) {

            // Reflection probability 
            float GGXreflect = DGGX(r, h, n);
            float pReflect = (1.0f - pLobe) * GGXreflect * hDotN / (4.0f * hDotWi);

            // Reflection weight
            glm::vec3 weight = glm::vec3(F / pReflect);

            // Return sample
            sample.weight = weight;
            sample.pdf = pReflect;
            sample.wo = dReflect;
			return sample;
        }

        // 2. Refract
        else {
            // TODO: h or n?
            //glm::vec3 dRefract = glm::refract(wo, h, eta);
            glm::vec3 dRefract = glm::refract(wi, h, eta);

            // Case 1: Total internal reflection
            if (glm::length(dRefract) == 0.0f) {

                // Reflection probability 
                float D = DGGX(r, h, n);
                float pReflect = (1.0f - pLobe) * D * hDotN / (4.0f * hDotWi * eta2);

                // Reflection weight
                glm::vec3 weight = glm::vec3(F / pReflect);

                glm::vec3 dReflect = glm::reflect(wi, h);
                sample.weight = weight;
                sample.pdf = pReflect;
                sample.wo = dReflect;
                return sample;
            }

            // Case 2: Refraction
            else {

                // Refraction probability 
                float D = DGGX(r, h, n);
                float pRefract = (1.0f - pLobe) * D * hDotN / (eta2 * glm::dot(dRefract, wi));

                // Refraction weight
				glm::vec3 weight = glm::vec3((1.0f - F) * eta * eta / pRefract);

                // Return sample
                sample.weight = weight;
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

    return matte->albedo / M_PI;
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
	float r = glm::max(0.05f, disney->roughness);
	glm::vec3 n = hit.front ? hit.n : -hit.n;
    float nDotWi = glm::max(0.0f, glm::dot(n, wi));
    float nDotWo = glm::max(0.0f, glm::dot(n, wo));
    if (nDotWi <= 0.0f || nDotWo <= 0.0f) return glm::vec3(0.0f); // Backface
    float hDotWi = glm::max(0.0f, glm::dot(h, wi));
    float hDotWo = glm::max(0.0f, glm::dot(h, wo));
   
    // Dielectric BRDF
    // TODO: Integrate subsurface scattering with dipole model
    float cos2 = hDotWo * hDotWo;
    float fd90 = 0.5f + 2.0f * disney->roughness * cos2;
    float fd = (1.0f + (fd90 - 1.0f) * (glm::pow((1.0f - nDotWi), 5))) * 
               (1.0f + (fd90 - 1.0f) * (glm::pow((1.0f - nDotWo), 5)));
    glm::vec3 diffuse = disney->albedo * M_1_PI * (1.0f - disney->metallic) * fd;

    // Specular BRDF
	glm::vec3 Fo = glm::mix(glm::vec3(0.04f), disney->albedo, disney->metallic);
    glm::vec3 F = Fo + (1.0f - Fo) * glm::pow(1.0f - hDotWo, 5.0f);
    float K = glm::pow((r + 1.0f),2) / 8.0f;
	float Gwi = nDotWi / (nDotWi * (1.0f - K) + K);
	float Gwo = nDotWo / (nDotWo * (1.0f - K) + K);
    float G = Gwi * Gwo;
	float D = DGGX(r, h, n);
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
	float r = glm::max(0.05f, disney->roughness);
	//glm::vec3 h = sampler.SampleHemisphereGGX(hit.n, r);
	glm::vec3 h = glm::normalize(wi + wo);
    float D = DGGX(r, h, hit.n);
	float hDotWi = glm::max(0.0f, glm::dot(h, wi));
	float hDotWo = glm::max(0.0f, glm::dot(h, wo));
    return D * hDotWi / (4.0f * hDotWo);
}