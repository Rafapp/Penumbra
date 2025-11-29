#include "shapes.h"

#define SPHERE_EPS 1e-6f

// === Ray intersections ===
bool Sphere::IntersectRay(const Ray& r, HitInfo& hit) {
    float a = glm::dot(r.d, r.d);
    float b = 2.0f * glm::dot(r.o, r.d);
    float c = glm::dot(r.o, r.o) - 1.0f;
    float discriminant = (b * b) - (4.0f * a * c);
    
    if (discriminant < 0.0f) return false;
    
    float sqrtD = sqrtf(discriminant);
    float aInv = 1.0f / (2.0f * a);
    float t0 = (-b - sqrtD) * aInv;
    float t1 = (-b + sqrtD) * aInv;
    
    float t = FLT_MAX;
    if (t0 > SPHERE_EPS) {
        t = t0;
        hit.front = true;
    }
    else if (t1 > SPHERE_EPS) {
        t = t1;
        hit.front = false;
    }
    else {
        return false;
    }
    
    // Compute hit point and normal in object space
    glm::vec3 objHitP = r.At(t);
    glm::vec3 objNormal = glm::normalize(objHitP);  // For unit sphere at origin
    
    // Transform to world space
    hit.p = glm::vec3(transform * glm::vec4(objHitP, 1.0f));
    hit.n = glm::normalize(glm::vec3(transform * glm::vec4(objNormal, 0.0f)));
    hit.t = t;
    hit.materialId = materialId;
    
    return true;
}

// === PBRT Conversion Constructors ===
Sphere::Sphere(minipbrt::Sphere* pbrtSphere) {
    this->transform = PbrtConverter::TransformToMat4(pbrtSphere->shapeToWorld);
    this->inverseTransform = glm::inverse(this->transform);
    this->materialId = static_cast<int>(pbrtSphere->material);
    this->areaLightId = static_cast<int>(pbrtSphere->areaLight);
    this->radius = pbrtSphere->radius;
}