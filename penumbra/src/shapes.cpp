#include "shapes.h"

// === Ray intersections ===
bool Sphere::IntersectRay(const Ray& r, HitInfo& hit) {
    // Ray-sphere intersection logic
    return false;
}

// === PBRT Conversion Constructors ===
glm::mat4 TransformToMat4(const minipbrt::Transform& transform) {
    glm::mat4 mat(
        glm::vec4(transform.start[0][0], transform.start[0][1], transform.start[0][2], transform.start[0][3]),
        glm::vec4(transform.start[1][0], transform.start[1][1], transform.start[1][2], transform.start[1][3]),
        glm::vec4(transform.start[2][0], transform.start[2][1], transform.start[2][2], transform.start[2][3]),
        glm::vec4(transform.start[3][0], transform.start[3][1], transform.start[3][2], transform.start[3][3])
    );
    return glm::transpose(mat);
}

Sphere::Sphere(minipbrt::Sphere* pbrtSphere) {
    this->transform = TransformToMat4(pbrtSphere->shapeToWorld);
    this->materialId = static_cast<int>(pbrtSphere->material);
    this->areaLightId = static_cast<int>(pbrtSphere->areaLight);
    this->radius = pbrtSphere->radius;
    std::cout << "Converted PBRT Sphere to Penumbra Sphere:" << std::endl;
    std::cout << "  Material ID: " << this->materialId << std::endl;
    std::cout << "  Area Light ID: " << this->areaLightId << std::endl;
    std::cout << "  Radius: " << this->radius << std::endl;
}