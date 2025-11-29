#include "lights.h"
#include "pbrtconverter.h"
#include <iostream>

PointLight::PointLight(minipbrt::PointLight* pbrtLight) {
    if (!pbrtLight) return;
    position = glm::vec3(pbrtLight->from[0], pbrtLight->from[1], pbrtLight->from[2]);  // Translation component
    this->intensity = glm::vec3(pbrtLight->I[0], pbrtLight->I[1], pbrtLight->I[2]);
}