#pragma once

#include "glm/glm.hpp"
#include "raytracing.h"
#include "materials.h"

class Light;

namespace Shading {
    glm::vec3 ShadeMaterial(const HitInfo& hit, Material* material, const std::vector<Light*>& lights);
    glm::vec3 ShadeMatte(const HitInfo& hit, MatteMaterial* matte, const std::vector<Light*>& lights);
}