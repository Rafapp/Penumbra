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

#include "tinybvh_glm_config.h"
#include "tiny_bvh.h"

static_assert(sizeof(tinybvh::bvhvec4) == sizeof(glm::vec4));
static_assert(alignof(tinybvh::bvhvec4) == alignof(glm::vec4));
static_assert(sizeof(uint32_t) == 4);

#include "minipbrt.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "raytracing.h"
#include "pbrtconverter.h"
#include "texture.h"

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
    glm::mat4 transform = glm::mat4(0.0f);
    glm::mat4 inverseTransform = glm::mat4(0.0f);
    glm::vec3 position = glm::vec3(0.0f);
    glm::vec3 scale = glm::vec3(0.0f);
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

struct SubMesh{
    tinybvh::BVH_SoA bvh;
    bool bvhReady = false;

    uint32_t nTris = 0;
    uint32_t nVerts = 0;

    std::vector<glm::vec4>* vertices = nullptr;
    std::vector<glm::uvec4>* triangles = nullptr;
    std::vector<glm::vec3>* normals = nullptr;
    std::vector<glm::vec3>* tangents = nullptr;
    std::vector<glm::vec3>* bitangents = nullptr;
    std::vector<glm::vec2>* uvs = nullptr;

    std::vector<uint32_t> bvhIndices;
    std::vector<glm::vec4> bvhTriSoup;

    uint32_t materialIndex = 0;
    bool BuildBVH();
};

class TriangleMesh : public Shape {
public:
    TriangleMesh(minipbrt::PLYMesh* plyMesh, Scene& scene, uint32_t shapeIdx);
    ~TriangleMesh();
    bool IntersectRay(const Ray& r, HitInfo& hit) override;
    std::vector<SubMesh*> meshes;
    
private:
    bool LoadMeshWithAssimp(const std::string& filename, Scene& scene, uint32_t shapeIdx);
    Texture* LoadTextureWithAssimp(aiMaterial* mat, aiTextureType type, const char* meshName);
    bool IntersectTriangle(const Ray& r, uint32_t triIdx, HitInfo& hit);
};