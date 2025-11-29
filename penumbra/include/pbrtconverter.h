#pragma once

#include "minipbrt.h"
#include "glm/glm.hpp"

class Scene;
class Camera;
class Shape;

namespace PbrtConverter {
    Scene ConvertScene(minipbrt::Scene* pbrtScene);
    Shape* ConvertShape(minipbrt::Shape* pbrtShape);
    Camera* ConvertCamera(minipbrt::Camera* pbrtCam);
    // Material* ConvertMaterial(minipbrt::Material* pbrtMat);
    // Light* ConvertLight(minipbrt::Light* pbrtLight);

    glm::mat4 TransformToMat4(const minipbrt::Transform& transform);
}