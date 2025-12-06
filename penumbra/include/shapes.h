#pragma once

#include <iostream>

#include "minipbrt.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "raytracing.h"
#include "pbrtconverter.h"

class Shape {
public:
    virtual ~Shape() = default;
    virtual bool IntersectRay(const Ray& r, HitInfo& hit) = 0;
    
    int GetMaterialId() const { return materialId; }
    int GetAreaLightId() const { return areaLightId; }
    glm::mat4 GetTransform() const { return transform; }
    glm::mat4 GetInverseTransform() const { return inverseTransform; }
    glm::vec3 GetPosition() const { return position; }
    glm::vec3 GetScale() const { return scale; }
    bool IsAreaLight() const { return areaLightId != minipbrt::kInvalidIndex; }
    AreaLight* areaLight = nullptr;
    Material* material = nullptr;
protected:
    glm::mat4 transform;
    glm::mat4 inverseTransform;
    glm::vec3 position;
    glm::vec3 scale;
    int materialId = -1;
    int areaLightId = -1;
    float surfaceArea = -1.0f;
};

class Sphere : public Shape {
public:
    Sphere(minipbrt::Sphere* pbrtSphere);
    bool IntersectRay(const Ray& r, HitInfo& hit) override;
    float GetRadius() const { return radius; }
private:
    float radius = 1.0f;
};

class TriangleMesh : public Shape {
public:
    TriangleMesh(minipbrt::PLYMesh* plyMesh);
    
    bool IntersectRay(const Ray& r, HitInfo& hit) override;
    
private:
    std::vector<glm::vec3> vertices;
    std::vector<glm::uvec3> triangles;
    std::vector<glm::vec3> normals;
    
    bool LoadMeshWithAssimp(const std::string& filename);
    bool IntersectTriangle(const Ray& r, uint32_t triIdx, HitInfo& hit);
};