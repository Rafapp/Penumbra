#pragma once

#include <iostream>

#define _USE_MATH_DEFINES
#include <cmath>

#ifndef M_PI 
#define M_PI 3.14159274101257324219f
#endif
#ifndef M_1_PI
#define M_1_PI 0.31830987334251403809f
#endif

#include "glm/glm.hpp"
#include "minipbrt.h"

#include "raytracing.h"

namespace minipbrt {
    struct PerspectiveCamera;
}

class Camera {
public:
    Camera() = default;
    virtual ~Camera() = default;

    virtual Ray GenerateRay(float x, float y, int width, int height) const = 0;

    glm::vec3 GetPosition() const { return position; }
    void SetPosition(const glm::vec3& pos) {
        position = pos;
        cameraToWorld[3] = glm::vec4(pos, 1.0f);
    }

    glm::vec3 GetDirection() const { return direction; }
    glm::vec3 GetRight() const { return right; }
    glm::vec3 GetUp() const { return up; }

protected:
    glm::vec3 position = glm::vec3(0.0f);
    glm::vec3 direction = glm::vec3(0.0f, 0.0f, 1.0f);
    glm::vec3 right = glm::vec3(1.0f, 0.0f, 0.0f);
    glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::mat4 cameraToWorld = glm::mat4(1.0f);
    glm::mat4 worldToCamera = glm::mat4(1.0f);
};

class PerspectiveCamera : public Camera {
public:
    PerspectiveCamera(minipbrt::PerspectiveCamera* pbrtCam);
    ~PerspectiveCamera() = default;

    Ray GenerateRay(float x, float y, int width, int height) const override;

    float GetFOV() const { return fov; }
    float GetFocalDistance() const { return focalDistance; }
    float GetAperture() const { return aperture; }

private:
    float fov = 40.0f;
    float focalDistance = 1.0f;
    float aperture = 0.0f;
};
