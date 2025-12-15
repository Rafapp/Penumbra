#pragma once
#include <glm/glm.hpp>

#define TINYBVH_USE_CUSTOM_VECTOR_TYPES

namespace tinybvh
{
    using bvhint2  = glm::ivec2;
    using bvhint3  = glm::ivec3;
    using bvhuint2 = glm::uvec2;
    using bvhuint3 = glm::uvec3;
    using bvhuint4 = glm::uvec4;
    using bvhvec2  = glm::vec2;
    using bvhvec3  = glm::vec3;
    using bvhvec4  = glm::vec4;
    using bvhdbl3  = glm::dvec3;
    using bvhmat4  = glm::mat4;
}
