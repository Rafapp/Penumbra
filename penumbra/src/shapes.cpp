#include "shapes.h"

#include <filesystem>

#define SPHERE_EPS 1e-8f
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
    else if (t1 > SPHERE_EPS * 10.0f) {
        t = t1;
        hit.front = false;
    }
    else {
        return false;
    }
    
    // Compute hit point, normal, and t value
    hit.p = glm::vec3(transform * glm::vec4(r.At(t), 1.0f));

    glm::vec3 nObj = glm::normalize(r.At(t));
    hit.n = glm::normalize(glm::vec3(transform * glm::vec4(nObj, 0.0f)));

    float s = glm::length(glm::vec3(transform * glm::vec4(r.d, 0.0f)));
    float tWorld = t * s;
    hit.t = tWorld;

    hit.materialId = materialId;
    hit.areaLightId = areaLightId;
    hit.shape = this;
    hit.material = material;

    return true;
}

bool TriangleMesh::IntersectRay(const Ray& r, HitInfo& hit) {
    // Möller & Trumbore, 1997
    // TODO: Bvh
    float closest = FLT_MAX;
    bool hitAny = false;

    for(int i = 0; i < nTris; i++){
        // TODO: ?
        glm::vec3 v0 = vertices->data()[triangles->data()[i].x];
        glm::vec3 e1 = vertices->data()[triangles->data()[i].y] - v0;
        glm::vec3 e2 = vertices->data()[triangles->data()[i].z] - v0;
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
            hit.p = glm::vec3(transform * glm::vec4(r.At(t), 1.0f));

            glm::vec3 n0 = normals->data()[triangles->data()[i].x];
            glm::vec3 n1 = normals->data()[triangles->data()[i].y];
            glm::vec3 n2 = normals->data()[triangles->data()[i].z];
            glm::vec3 nObj = glm::normalize((1 - u - v)*n0 + u*n1 + v*n2);
            glm::mat3 normalMatrix = glm::transpose(glm::mat3(inverseTransform));
            hit.n = glm::normalize(normalMatrix * nObj);

            float s = glm::length(glm::vec3(transform * glm::vec4(r.d, 0.0f)));
            float tWorld = t * s;
            hit.t = tWorld;

            hit.front = glm::dot(nObj, -r.d) > 0.0f;
            hit.materialId = materialId;
            hit.areaLightId = areaLightId;
            hit.shape = this;
            hit.material = material;
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
        this->surfaceArea = 4.0f *M_PI * radius * radius;
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
    //     this->surfaceArea = 4.0f *M_PI * radius * radius;
    // }
}

// === Mesh loading with Assimp ===
bool TriangleMesh::LoadMeshWithAssimp(const std::string& filename){
    std::cout << "Loading mesh with Assimp: " << filename << std::endl;
    Assimp::Importer importer;
    // NOTE: Assuming flipped winding order for PBRT
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
    nVerts = scene->mMeshes[0]->mNumVertices;
    nTris = scene->mMeshes[0]->mNumFaces;
    if(nVerts > 0 && nTris > 0){
        vertices = new std::vector<glm::vec3>(nVerts);
        for (uint32_t i = 0; i < nVerts; i++) {
            (*vertices)[i] = glm::vec3(
                mesh->mVertices[i].x,
                mesh->mVertices[i].y,
                mesh->mVertices[i].z
            );
        }
    } else {
        std::cerr << "Face or Vertex count for mesh < 0" << std::endl;
    }
    
    // Copy normals
    if (mesh->HasNormals()) {
        normals = new std::vector<glm::vec3>(nVerts);
        for (size_t i = 0; i < nVerts; i++) {
            (*normals)[i] = glm::vec3(mesh->mNormals[i].x,
                                      mesh->mNormals[i].y,
                                      mesh->mNormals[i].z);
        }
    } else {
        std::cerr << "  Warning: No normals found for mesh ..." << std::endl;
    }
    
    // Copy triangles (triangle indices) 
    triangles = new std::vector<glm::uvec3>(nTris); 
    for (size_t i = 0; i < nTris; i++) {
        const aiFace& face = mesh->mFaces[i];
        (*triangles)[i] = glm::uvec3(face.mIndices[0],
                                      face.mIndices[1],
                                      face.mIndices[2]);
    }
    
    std::cout << "Loaded mesh: " << filename << std::endl;
    std::cout << "  Vertices: " << nVerts << std::endl;
    std::cout << "  Triangles: " << nTris << std::endl;

    std::cout << "Building BVH ..." << std::endl;
    BuildBVH();
    std::cout << "  BVH Complete" << std::endl;
    return true;
}

// === BVH Construction ===
bool TriangleMesh::BuildBVH(){
    // bvh.Build((tinybvh::bvhvec4*)vertices, nVerts, (tinybvh::bvhuint4*)triangles, nTris);
    bvh.Build((tinybvh::bvhvec4*)vertices, nVerts);
}