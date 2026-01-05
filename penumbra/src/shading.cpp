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
    const float EPS = 1e-4f;
	const float NEPS = 1.0f - EPS;

    glm::vec3 albedo = disney->GetAlbedo(hit.uv);
    float roughness = disney->GetRoughness(hit.uv);
    float metallic = disney->GetMetallic(hit.uv);

    float r = roughness;
    float r2 = r * r;
	glm::vec3 n = hit.front ? hit.n : -hit.n;
    glm::vec3 I = -wi;
	float eta = hit.front ? 1.0f / disney->eta : disney->eta;
	float eta2 = eta * eta;

    // =================	
    // === (A) Glass ===
    // =================
    // TODO: Bug, had this
    // if (metallic < EPS && roughness < EPS && disney->eta > 1.0f) {
    if (metallic < EPS && roughness < EPS && eta > 1.0f) {
		float nDotI = glm::dot(n, I);
		float F = ShlickFresnel(nDotI, eta);
        F = glm::clamp(F, 1e-4f, 1.0f - 1e-4f);

        // 1. Reflect
        if (sampler.Sample1D() > F) {
			sample.wo = glm::reflect(I, n);
            sample.pdf = F;
            sample.weight = glm::vec3(1.0f);
            sample.isDelta = true;
            return sample;
        }
        // 2. Refract
        else {

			// Case 1: Total internal reflection
			glm::vec3 dRefract = glm::refract(I, n, eta);
            if (glm::length(dRefract) == 0.0f) {
				sample.wo = glm::reflect(I, n);
                sample.pdf = 1.0f;
				sample.weight = glm::vec3(1.0f);
                sample.isDelta = true;
                return sample;
            }

            // Case 2: Refraction
			sample.wo = dRefract;
			sample.pdf = 1.0f - F;
			sample.weight = glm::vec3(eta * eta);
			sample.isDelta = true;
			return sample;
        }
    }

    // =========================
    // === (B) Perfect metal === 
    // =========================
    else if (metallic > NEPS && roughness < EPS) {
        glm::vec3 dReflect = glm::reflect(I, n);
        sample.wo = dReflect;
        sample.weight = albedo;
        sample.pdf = 1.0f;
        sample.isDelta = true;
        return sample;
    }

    // ========================
    // === (C) Glossy Metal ===
    // ========================
    else if (metallic > NEPS && roughness > EPS) {

        glm::vec3 h = glm::vec3(0.0f);
        float hDotI = -1.0f;

        // Guarantee GGX sample in visible hemisphere | TODO: Implement Heiz VNDF
        int maxTrials = 10;
        int trials = 0;
        do {
            h = sampler.SampleHemisphereGGX(n, r);
            hDotI = glm::dot(h, I);
        } while (hDotI <= 0.0f && ++trials < maxTrials);

        glm::vec3 dReflect = glm::reflect(I, h);
		float nDotWo = glm::dot(n, dReflect);
        if (hDotI <= 0.0f || nDotWo <= 0.0f) { sample.pdf = 0;  return sample; }

        float hDotN = glm::dot(h, n);
        float hDotWo = glm::abs(glm::dot(dReflect, h));
        if (hDotWo <= 1e-6f) { sample.pdf = 0; return sample; }

		// Reflection probability 
		float D = DGGX(r, h, n);
		float pReflect = D * hDotN / (4.0f * hDotWo);

		// Return sample
		float cos = glm::max(0.0f, glm::dot(n, dReflect));
		sample.weight = glm::vec3(cos / pReflect);
		sample.pdf = pReflect;
		sample.wo = dReflect;
		return sample;
    }

    // ===================
    // === (D) Plastic ===
    // ===================
    else {
		float Fo = 0.04f;
		float pDiffuse = 1.0f - Fo;
        float pSpecular = Fo;

        // --- I. DIFFUSE LOBE ---
        if (sampler.Sample1D() < pDiffuse) {
            sample.wo = sampler.SampleHemisphereCosine(n);
            float cos = glm::max(0.0f, glm::dot(n, sample.wo));
            sample.pdf = pDiffuse * cos * M_1_PI;
            sample.weight = glm::vec3(cos / pDiffuse);
            return sample;
        }

        // --- II. SPECULAR LOBE ---
        glm::vec3 h = glm::vec3(0.0f);
        float hDotI = -1.0f;

        // Guarantee GGX sample in visible hemisphere | TODO: Implement Heiz VNDF
        int maxTrials = 10;
        int trials = 0;
        do {
            h = sampler.SampleHemisphereGGX(n, r);
            hDotI = glm::dot(h, I);
        } while (hDotI <= 0.0f && ++trials < maxTrials);

        glm::vec3 dReflect = glm::reflect(I, h);
		float nDotWo = glm::dot(n, dReflect);
        if (hDotI <= 0.0f || nDotWo <= 0.0f) { sample.pdf = 0.0f; return sample; }

        float hDotN = glm::dot(h, n);
		float hDotWo = glm::abs(glm::dot(dReflect, h));
        if (hDotWo <= 1e-6f) { sample.pdf = 0; return sample; }

		// Reflection probability 
		float D = DGGX(r, h, n);
		float pReflect = pSpecular * D * hDotN / (4.0f * hDotWo);

		// Return sample
		sample.wo = dReflect;
		float cos = glm::max(0.0f, glm::dot(n, sample.wo));
		sample.pdf = pReflect;
		sample.weight = glm::vec3(cos / sample.pdf);
		return sample;
    }
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

    return matte->albedo / float(M_PI);
}

// "Physically-Based Shading at Disney," Brent Burley (2012)
// "Extending the Disney BRDF to a BSDF with Integrated Subsurface Scattering," Brent Burley (2015)
// TODO: Sampler may not be needed
glm::vec3 Shading::ShadeDisney(const HitInfo& hit, 
                               const glm::vec3& wi, 
                               const glm::vec3& wo, 
                               const DisneyMaterial* disney) {

	glm::vec3 n = hit.front ? hit.n : -hit.n;
    float etaI = hit.front ? 1.0f : disney->eta;
    float etaO = hit.front ? disney->eta : 1.0f;
    float eta = etaI / etaO;
    float nDotWi = glm::dot(n, wi);
    float nDotWo = glm::dot(n, wo);

    bool isReflection = (nDotWi > 0.0f && nDotWo > 0.0f);
    bool isTransmission = (nDotWi * nDotWo < 0.0f);
    if (!isReflection && !isTransmission) return glm::vec3(0);

    glm::vec3 albedo = disney->GetAlbedo(hit.uv);
    float roughness = disney->GetRoughness(hit.uv);
    float metallic = disney->GetMetallic(hit.uv);

    // BSSRDF
    if (isReflection) {
		nDotWi = glm::max(0.0f, nDotWi);
		nDotWo = glm::max(0.0f, nDotWo);
		glm::vec3 h = glm::normalize(wi + wo);
		float hDotWi = glm::abs(glm::dot(h, wi));
		float hDotWo = glm::abs(glm::dot(h, wo));

		// Diffuse | TODO: Integrate subsurface scattering with dipole model
		float cos2 = hDotWo * hDotWo;
		float fd90 = 0.5f + 2.0f * roughness * cos2;
		float fd = (1.0f + (fd90 - 1.0f) * (glm::pow((1.0f - nDotWi), 5))) * 
				   (1.0f + (fd90 - 1.0f) * (glm::pow((1.0f - nDotWo), 5)));
		glm::vec3 diffuse = albedo * float(M_1_PI) * (1.0f - metallic) * fd;

		// Specular
		glm::vec3 Fo = glm::mix(glm::vec3(0.04f), albedo, metallic);
		glm::vec3 F = Fo + (1.0f - Fo) * glm::pow(1.0f - hDotWo, 5.0f);
		float K = glm::pow((roughness + 1.0f),2) / 8.0f;
		float Gwi = nDotWi / (nDotWi * (1.0f - K) + K);
		float Gwo = nDotWo / (nDotWo * (1.0f - K) + K);
		float G = Gwi * Gwo;
		float D = DGGX(roughness, h, n);
		glm::vec3 specular = (F * G * D) / (4.0f * nDotWi * nDotWo);

        return diffuse + specular;
    }

    // BTDF
    else if (isTransmission) {
		glm::vec3 h = glm::normalize(etaI * wi + etaO * wo);
        if (glm::dot(h, n) < 0.0f) h = -h;
		float hDotWi = glm::abs(glm::dot(h, wi));
		float hDotWo = glm::abs(glm::dot(h, wo));
		nDotWi = glm::abs(nDotWi);
		nDotWo = glm::abs(nDotWo);
		float D = DGGX(roughness, h, n);
		float K = glm::pow((roughness + 1.0f), 2) / 8.0f;
		float Gwi = nDotWi / (nDotWi * (1.0f - K) + K);
		float Gwo = nDotWo / (nDotWo * (1.0f - K) + K);
		float G = Gwi * Gwo;

		float F = ShlickFresnel(hDotWi, etaI / etaO);
        float denom = etaI * hDotWi + etaO * hDotWo;
        float factor = (etaO * etaO) * hDotWi * hDotWo / (nDotWi * nDotWo * denom * denom);
        return albedo * (1.0f - F) * D * G * factor;
    }

    // Invalid
    else return glm::vec3(0.0f);
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

    glm::vec3 albedo = disney->GetAlbedo(hit.uv);
    float roughness = disney->GetRoughness(hit.uv);
    float metallic = disney->GetMetallic(hit.uv);

    float EPS = 1e-4f;
	// TODO: Need eta check here?
    bool isDelta = roughness < EPS &&
            (metallic > 1.0f - EPS ||
		     disney->eta > 1.0f);
    if (isDelta) return 0.0f;

	glm::vec3 n = hit.front ? hit.n : -hit.n;
	float nDotWi = glm::dot(n, wi);
	float nDotWo = glm::dot(n, wo);

    bool isTransmission = (nDotWi <= 0.0f || nDotWo <= 0.0f);
    if (isTransmission) return 0.0f;

    float Fo = glm::mix(0.04f, 1.0f, metallic);
    float pSpecular = Fo;
    float pDiffuse = 1.0f - pSpecular;

	float pdf = 0.0f;

	// --- I. DIFFUSE LOBE ---
	float pdfDiffuse = nDotWi * M_1_PI;
	pdf += pDiffuse * pdfDiffuse;

	// --- II. SPECULAR LOBE ---
	glm::vec3 h = glm::normalize(wi + wo);
	float hDotN = glm::max(0.0f, glm::dot(h, n));
	float hDotWo = glm::abs(glm::dot(h, wo));

	if (hDotN > 0.0f && hDotWo > 1e-6f) {
		float D = DGGX(roughness, h, n);
		float pdfSpecular = D * hDotN / (4.0f * hDotWo);
		pdf += pSpecular * pdfSpecular;
	}

    return pdf;
}