#include "shapes.h"

#include <filesystem>

#define SPHERE_EPS 1e-4f
#define TRI_EPS 1e-4f

// === Ray intersections ===
bool Sphere::IntersectRay(const Ray& r, HitInfo& hit) {
    float a = glm::dot(r.d, r.d);
    float b = 2.0f * glm::dot(r.o, r.d);
    float c = glm::dot(r.o, r.o) - 1.0f;
    float discriminant = (b * b) - (4.0f * a * c);
    
    if (discriminant < 0.0f) return false;
    
    float sqrtD = sqrtf(discriminant);
    float aInv = 1.0f / (2.0f * a);
    float t0 = (-b - sqrtD) * aInv;
    float t1 = (-b + sqrtD) * aInv;
    
    float t = FLT_MAX;
    if (t0 > SPHERE_EPS) {
        t = t0;
        hit.front = true;
    }
    else if (t1 > SPHERE_EPS) {
        t = t1;
        hit.front = false;
    }
    else {
        return false;
    }
    
    // Compute hit point, normal, and t value in world space
    hit.p = glm::vec3(transform * glm::vec4(r.At(t), 1.0f));
    glm::vec3 nObj = glm::normalize(r.At(t));
    hit.n = glm::normalize(glm::vec3(transform * glm::vec4(nObj, 0.0f)));
    hit.t = t;
    hit.materialId = materialId;
    hit.areaLightId = areaLightId;
    
    return true;
}

bool TriangleMesh::IntersectRay(const Ray& r, HitInfo& hit) {
    // Möller & Trumbore, 1997
    // TODO: Bvh
    int nTris = static_cast<int>(triangles.size());
    float closest = FLT_MAX;
    bool hitAny = false;

    for(int i = 0; i < nTris; i++){
        glm::vec3 v0 = vertices[triangles[i].x];
        glm::vec3 e1 = vertices[triangles[i].y] - v0;
        glm::vec3 e2 = vertices[triangles[i].z] - v0;
        glm::vec3 ray_cross_e2 = glm::cross(r.d, e2);
        float det = glm::dot(e1, ray_cross_e2);

        // Cull back-facing triangles
        if (fabs(det) < TRI_EPS) continue;

        float invDet = 1.0f / det;
        glm::vec3 pToV0 = r.o - v0;

        float u = invDet * glm::dot(pToV0, ray_cross_e2);
        if (u < 0 || u > 1) continue;
        glm::vec3 pToV0_cross_e1 = glm::cross(pToV0,e1);

        float v = invDet * glm::dot(r.d, pToV0_cross_e1);
        if (v < 0 || u + v > 1) continue;

        float t = invDet * glm::dot(e2, pToV0_cross_e1);
        if(t < TRI_EPS) continue;

        hitAny = true;

        if(t < closest){
            closest = t;

            // Compute hit point, normal, and t value in world space
            hit.p = transform * glm::vec4(r.At(t), 1.0f);
            glm::vec3 n0 = normals[triangles[i].x];
            glm::vec3 n1 = normals[triangles[i].y];
            glm::vec3 n2 = normals[triangles[i].z];
            glm::vec3 nObj = glm::normalize((1 - u - v)*n0 + u*n1 + v*n2);
            glm::mat3 normalMatrix = glm::transpose(glm::mat3(inverseTransform));
            hit.n = glm::normalize(normalMatrix * nObj);
            hit.t = t;
            hit.front = glm::dot(hit.n, -r.d) > 0.0f;
            hit.materialId = materialId;
            hit.areaLightId = areaLightId;
        }
    }
    return hitAny;
}

// === PBRT Conversion Constructors ===
Sphere::Sphere(minipbrt::Sphere* pbrtSphere) {
    this->transform = PbrtConverter::TransformToMat4(pbrtSphere->shapeToWorld);
    this->inverseTransform = glm::inverse(this->transform);
    this->position = glm::vec3(this->transform[3]);
    this->scale = glm::vec3(
        glm::length(glm::vec3(this->transform[0])),
        glm::length(glm::vec3(this->transform[1])),
        glm::length(glm::vec3(this->transform[2])));
    this->materialId = static_cast<int>(pbrtSphere->material);
    this->areaLightId = static_cast<int>(pbrtSphere->areaLight);
    this->radius = pbrtSphere->radius;

    // Calculate surface area if sphere is area light
    if(areaLightId != minipbrt::kInvalidIndex) {
        this->surfaceArea = 4.0f * M_PI * radius * radius;
    }
}

TriangleMesh::TriangleMesh(minipbrt::PLYMesh* plyMesh) {
    if (!plyMesh) return;
    
    // Load mesh via Assimp
    std::string meshPath = plyMesh->filename;
    size_t pos = meshPath.find("./resources/meshes/");
    if (pos != std::string::npos) {
        // Minipbrt works from the scenes dir, so we need to clean the path
        meshPath = meshPath.substr(pos);
    }
    if (!LoadMeshWithAssimp(meshPath)) {
        std::cerr << "Failed to load mesh: " << meshPath << std::endl;
        return;
    }
    this->transform = PbrtConverter::TransformToMat4(plyMesh->shapeToWorld);
    this->inverseTransform = glm::inverse(this->transform);
    this->position = glm::vec3(this->transform[3]);
    this->scale = glm::vec3(
        glm::length(glm::vec3(this->transform[0])),
        glm::length(glm::vec3(this->transform[1])),
        glm::length(glm::vec3(this->transform[2])));
    this->materialId = static_cast<int>(plyMesh->material);
    this->areaLightId = static_cast<int>(plyMesh->areaLight);

    // TODO: Calculate surface area if mesh is area light
    // if(areaLightId != minipbrt::kInvalidIndex) {
    //     this->surfaceArea = 4.0f * M_PI * radius * radius;
    // }
}

// === Mesh loading with Assimp ===
bool TriangleMesh::LoadMeshWithAssimp(const std::string& filename){
    std::cout << "Loading mesh with Assimp: " << filename << std::endl;
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(filename,
        aiProcess_Triangulate | 
        aiProcess_GenNormals | 
        aiProcess_FlipWindingOrder);
    
    if (!scene || scene->mNumMeshes == 0) {
        std::cerr << "Failed to load mesh: " << filename << std::endl;
        std::cerr << "Error: " << importer.GetErrorString() << std::endl;
        return false;
    }
    
    // Load first mesh
    const aiMesh* mesh = scene->mMeshes[0];
    
    // Copy vertices
    vertices.resize(mesh->mNumVertices);
    for (size_t i = 0; i < mesh->mNumVertices; i++) {
        vertices[i] = glm::vec3(mesh->mVertices[i].x,
                                     mesh->mVertices[i].y,
                                     mesh->mVertices[i].z);
    }
    
    // Copy normals
    if (mesh->HasNormals()) {
        normals.resize(mesh->mNumVertices);
        for (size_t i = 0; i < mesh->mNumVertices; i++) {
            normals[i] = glm::vec3(mesh->mNormals[i].x,
                                        mesh->mNormals[i].y,
                                        mesh->mNormals[i].z);
        }
    }
    
    // Copy faces (triangles)
    triangles.resize(mesh->mNumFaces);
    for (size_t i = 0; i < mesh->mNumFaces; i++) {
        const aiFace& face = mesh->mFaces[i];
        if (face.mNumIndices == 3) {
            triangles[i] = glm::uvec3(face.mIndices[0],
                                           face.mIndices[1],
                                           face.mIndices[2]);
        }
    }
    
    std::cout << "Loaded mesh: " << filename << std::endl;
    std::cout << "  Vertices: " << vertices.size() << std::endl;
    std::cout << "  Triangles: " << triangles.size() << std::endl;

    return true;
}