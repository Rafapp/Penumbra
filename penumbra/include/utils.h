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

namespace Utils {
	void Orthonormals(const glm::vec3& n, glm::vec3& b1, glm::vec3& b2);
}