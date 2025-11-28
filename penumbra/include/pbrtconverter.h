#pragma once

#include "minipbrt.h"

#include "scene.h"

namespace PbrtConverter {
    Scene ConvertScene(minipbrt::Scene* pbrtScene);
    Shape* ConvertShape(minipbrt::Shape* pbrtShape);
    // Material* ConvertMaterial(minipbrt::Material* pbrtMat);
    // Camera* ConvertCamera(minipbrt::Camera* pbrtCam);
    // Light* ConvertLight(minipbrt::Light* pbrtLight);
}