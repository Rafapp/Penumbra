#include "utils.h"

void Utils::Orthonormals(const glm::vec3& n, glm::vec3& b1, glm::vec3& b2) {
    if (fabs(n.x) > fabs(n.z)) {
        b1 = glm::normalize(glm::vec3(-n.y, n.x, 0.0f));
    }
    else {
        b1 = glm::normalize(glm::vec3(0.0f, -n.z, n.y));
    }
    b2 = glm::cross(n, b1);
}
