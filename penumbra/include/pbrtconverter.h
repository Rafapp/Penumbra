#pragma once

#include "minipbrt.h"
#include "glm/glm.hpp"

class Scene;
class Camera;
class Shape;
class Light;
class Material;

namespace PbrtConverter {
    Scene ConvertScene(minipbrt::Scene* pbrtScene);
    Shape* ConvertShape(minipbrt::Shape* pbrtShape);
    Camera* ConvertCamera(minipbrt::Camera* pbrtCam);
    Light* ConvertLight(minipbrt::Light* pbrtLight);
    Material* ConvertMaterial(minipbrt::Material* pbrtMat);

    glm::mat4 TransformToMat4(const minipbrt::Transform& transform);
}