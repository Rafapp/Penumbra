#include "camera.h"
#include "pbrtconverter.h"

Ray PerspectiveCamera::GenerateRay(float u, float v, int width, int height) const {
    // Generate a ray from the camera through pixel (x, y) in the view plane
	float fov = (fov * M_PI) / 180.0f;

	float h = 2.0f * focalDistance * tanf(fov / 2.0f);
	float w = h * (width / height);

	float x = -(w / 2.0f) + (w * u / width);
	float y = (h / 2.0f) - (h * v / height);

    // Right handed system, camera looks at -z
	float z = -focalDistance;

    return Ray(glm::vec3(0.0f), glm::normalize(glm::vec3(x, y, z)));
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