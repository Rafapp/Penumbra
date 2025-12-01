#include <iostream>

#include "lights.h"
#include "pbrtconverter.h"
#include "renderer.h"

#define SHADOW_EPS 1e-4f

PointLight::PointLight(minipbrt::PointLight* pbrtLight) {
    if (!pbrtLight) return;
    position = glm::vec3(pbrtLight->from[0], pbrtLight->from[1], pbrtLight->from[2]);  // Translation component
    this->intensity = glm::vec3(pbrtLight->I[0], pbrtLight->I[1], pbrtLight->I[2]);
}

float PointLight::Illuminate(const HitInfo& hit, Renderer& renderer) {
    glm::vec3 toLight = position - hit.p;
    float dist = glm::length(toLight);
    Ray shadowRay(hit.p + hit.n * SHADOW_EPS, glm::normalize(toLight));
    return renderer.TraceShadowRay(shadowRay, dist);
}

AreaLight::AreaLight(minipbrt::AreaLight* pbrtLight) {
    if (!pbrtLight) return;
    pbrtLight->scale;
    pbrtLight->type();
}