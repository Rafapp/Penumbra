#include "camera.h"
#include "pbrtconverter.h"

Ray PerspectiveCamera::GenerateRay(float u, float v, int width, int height) const {
    float camFov = (fov * M_PI) / 180.0f;
    float h = 2.0f * focalDistance * tanf(camFov / 2.0f);
    float w = h * (width / (float)height);
    float x = -(w / 2.0f) + (w * u / width);
    float y = (h / 2.0f) - (h * v / height);
    float z = focalDistance;
    
    glm::vec4 dirCam = glm::normalize(glm::vec4(x, y, z, 0.0f));
    glm::vec4 oWorld = cameraToWorld * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
    glm::vec4 dWorld = cameraToWorld * dirCam;
    return Ray(oWorld, glm::normalize(glm::vec3(dWorld)));
}

// === PBRT Conversion Constructors ===
PerspectiveCamera::PerspectiveCamera(minipbrt::PerspectiveCamera* pbrtCam) {
    if (!pbrtCam) return;
    
    this->cameraToWorld = PbrtConverter::TransformToMat4(pbrtCam->cameraToWorld);
    this->worldToCamera = glm::inverse(this->cameraToWorld);

    // Extract position, direction, up from transform
    this->position = glm::vec3(this->cameraToWorld[3]);  // Translation column
    this->direction = glm::normalize(glm::vec3(this->cameraToWorld[2]));  // Forward (Z axis)
    this->up = glm::normalize(glm::vec3(this->cameraToWorld[1]));  // Up (Y axis)

    // Recompute right
    this->right = glm::normalize(glm::cross(this->direction, this->up));
    this->up = glm::normalize(glm::cross(this->right, this->direction));
    
    // Camera-specific parameters
    this->fov = pbrtCam->fov;
    this->focalDistance = pbrtCam->focaldistance;
    this->aperture = pbrtCam->lensradius;
}