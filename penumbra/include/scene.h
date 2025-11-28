#pragma once

#include <vector>

#include "minipbrt.h"

#include "camera.h"
#include "shapes.h"
#include "lights.h"
#include "materials.h"

struct Scene {
    float startTime = 0.0f;
    float endTime = 0.0f;

    Camera* camera = nullptr;
    std::vector<Shape*> shapes;
    std::vector<Light*> lights;
    std::vector<Material*> materials;
};