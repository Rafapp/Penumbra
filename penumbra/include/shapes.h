#pragma once

#include <iostream>

#include "minipbrt.h"

#include "raytracing.h"

class Shape {
public:
    virtual ~Shape() = default;
    virtual bool IntersectRay(const Ray& r, HitInfo& hit) = 0;
    
    int GetMaterialId() const { return materialId; }
    int GetAreaLightId() const { return areaLightId; }
    glm::mat4 GetTransform() const { return transform; }
    
protected:
    glm::mat4 transform;
    int materialId = -1;
    int areaLightId = -1;
};

class Sphere : public Shape {
public:
    Sphere(minipbrt::Sphere* pbrtSphere);
    bool IntersectRay(const Ray& r, HitInfo& hit) override;
private:
    float radius = 1.0f;
};