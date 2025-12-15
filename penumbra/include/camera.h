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

class Camera {
public:
    virtual ~Camera() = default;
    virtual Ray GenerateRay(float x, float y, int width, int height) const = 0;
    
    glm::vec3 GetPosition() const { return position; }
    glm::vec3 GetDirection() const { return direction; }
    glm::vec3 GetUp() const { return up; }
    
protected:
    glm::vec3 position;
    glm::vec3 direction;
    glm::vec3 up;
    glm::vec3 right;
    glm::mat4 cameraToWorld;
    glm::mat4 worldToCamera;
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