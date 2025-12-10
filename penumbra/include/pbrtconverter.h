#pragma once

#include <cmath>
#include "minipbrt.h"
#include "glm/glm.hpp"

class Scene;
class Camera;
class Shape;
class Material;
class TriangleMesh;
class AreaLight;
class IdealLight;

namespace PbrtConverter {
    Scene ConvertScene(minipbrt::Scene* pbrtScene);
    Shape* ConvertShape(minipbrt::Shape* pbrtShape);
    Camera* ConvertCamera(minipbrt::Camera* pbrtCam);
    IdealLight* ConvertIdealLight(minipbrt::Light* pbrtLight);
    AreaLight* ConvertAreaLight(minipbrt::AreaLight* pbrtAreaLight);
    Material* ConvertMaterial(minipbrt::Material* pbrtMat);

    glm::mat4 TransformToMat4(const minipbrt::Transform& transform);
}