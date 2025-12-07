#pragma once

#include "glm/glm.hpp"
#include "raytracing.h"
#include "materials.h"
#include "sampling.h"
#include "lights.h"

// TODO: Do we need a namespace or should we have a class?
namespace Shading {
    struct BxDFSample {
        glm::vec3 d;  // Outgoing direction
        glm::vec3 color;      // f(wi, wo) * cos(theta)
        float pdf;            // Probability of this sample
    };

    BxDFSample SampleMaterial(const HitInfo& hit, const Material* material, const glm::vec3& wi, Sampler& sampler);
    BxDFSample SampleMatte(const HitInfo& hit, const MatteMaterial* matte, const glm::vec3& wi, Sampler& sampler);

    glm::vec3 ShadeMaterial(const HitInfo& hit, const glm::vec3& wi, const Material* material);
    glm::vec3 ShadeMatte(const HitInfo& hit, const glm::vec3& wi, const MatteMaterial* matte);

    float PdfMaterial(const HitInfo& hit, const glm::vec3& wi, const Material* mat);
    float PdfMatte(const HitInfo& hit, const MatteMaterial* matte, const glm::vec3& wi);
}