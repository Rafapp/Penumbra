#pragma once

#define _USE_MATH_DEFINES
#include <cmath>

#ifndef M_PI 
#define M_PI 3.14159274101257324219f
#endif
#ifndef M_1_PI
#define M_1_PI 0.31830987334251403809f
#endif

#include "glm/glm.hpp"
#include "raytracing.h"
#include "materials.h"
#include "sampling.h"
#include "lights.h"

// TODO: Do we need a namespace or should we have a class?
namespace Shading {
    struct BxDFSample {
        glm::vec3 wo = glm::vec3(0.0f);     // Outgoing direction
        glm::vec3 weight = glm::vec3(0.0f);  // BxDF(hit, wi, wo, mat);
        float pdf = 0.0f;                   // Probability of this sample
        bool isDelta = false;
    };

    // TODO: Create context struct for arguments, too many ...
    BxDFSample SampleMaterial(const HitInfo& hit, 
                              const glm::vec3& wi, 
                              const Material* material, 
                              Sampler& sampler);

    BxDFSample SampleMatte(const HitInfo& hit, 
                           const glm::vec3& wi, 
                           const MatteMaterial* matte, 
                           Sampler& sampler);

    BxDFSample SampleDisney(const HitInfo& hit, 
                            const glm::vec3& wi, 
                            const DisneyMaterial* disney, 
                            Sampler& sampler);

    glm::vec3 ShadeMaterial(const HitInfo& hit, 
                            const glm::vec3& wi, 
                            const glm::vec3& wo,
                            const Material* material);

    glm::vec3 ShadeMatte(const HitInfo& hit, 
                         const glm::vec3& wi,
                         const glm::vec3& wo, 
                         const MatteMaterial* matte);

    glm::vec3 ShadeDisney(const HitInfo& hit, 
                          const glm::vec3& wi, 
                          const glm::vec3& wo, 
                          const DisneyMaterial* disney);

    float PdfMaterial(const HitInfo& hit, 
                      const glm::vec3& wi, 
                      const glm::vec3& wo,
                      const Material* mat,
                      Sampler& sampler);

    float PdfMatte(const HitInfo& hit, 
                   const glm::vec3& wi,
                   const glm::vec3& wo,
                   const MatteMaterial* matte,
                   Sampler& sampler); 

    float PdfDisney(const HitInfo& hit, 
                   const glm::vec3& wi,
                   const glm::vec3& wo,
                   const DisneyMaterial* disney,
                   Sampler& sampler);
}