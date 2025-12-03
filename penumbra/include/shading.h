#pragma once

#include "glm/glm.hpp"
#include "raytracing.h"
#include "materials.h"
#include "sampling.h"
#include "lights.h"

namespace Shading {
    struct BxDFSample {
        glm::vec3 direction;  // Outgoing direction
        glm::vec3 color;      // f(wi, wo) * cos(theta)
        float pdf;            // Probability of this sample
    };

    BxDFSample SampleMaterial(const HitInfo& hit, Material* material, const glm::vec3& wi, Sampler& sampler);
    BxDFSample SampleMatte(const HitInfo& hit, MatteMaterial* matte, const glm::vec3& wi, Sampler& sampler);

    glm::vec3 ShadeMaterial(const HitInfo& hit, Material* material, const std::vector<Light*>& lights);
    glm::vec3 ShadeMatte(const HitInfo& hit, MatteMaterial* matte, const std::vector<Light*>& lights);
}