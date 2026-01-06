#include "shapes.h"

#include <filesystem>

#include "materials.h"
#include "scene.h"

#define SPHERE_EPS 1e-8f
#define TRI_EPS 1e-6f

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
    // TODO: TLAS
    bool hitAny = false;
    float closest = FLT_MAX;

    unsigned int nMeshes = meshes.size();
    for(int i = 0; i < nMeshes; i++){
        SubMesh* mesh = meshes[i];
        if (!mesh->bvhReady) continue;

        float s = glm::length(glm::vec3(transform * glm::vec4(r.d, 0.0f)));
        if (!(s > 0.0f)) continue;

        float tMaxObj = hit.t / s;
        if (!(tMaxObj > 0.0f)) tMaxObj = FLT_MAX;

        tinybvh::Ray ray(r.o, r.d, tMaxObj);
        mesh->bvh.Intersect(ray);

        if (ray.hit.t >= tMaxObj) continue;

        uint32_t idx = ray.hit.prim;
        if (idx >= mesh->nTris) continue;

        const glm::vec3 v0 = glm::vec3(mesh->bvhTriSoup[3ull * idx + 0]);
        const glm::vec3 v1 = glm::vec3(mesh->bvhTriSoup[3ull * idx + 1]);
        const glm::vec3 v2 = glm::vec3(mesh->bvhTriSoup[3ull * idx + 2]);

        glm::vec3 e1 = v1 - v0;
        glm::vec3 e2 = v2 - v0;

        glm::vec3 pvec = glm::cross(r.d, e2);
        float det = glm::dot(e1, pvec);
        if (fabs(det) < TRI_EPS) continue;

        float invDet = 1.0f / det;
        glm::vec3 tvec = r.o - v0;

        float u = invDet * glm::dot(tvec, pvec);
        if (u < 0.0f || u > 1.0f) continue;

        glm::vec3 qvec = glm::cross(tvec, e1);
        float v = invDet * glm::dot(r.d, qvec);
        if (v < 0.0f || u + v > 1.0f) continue;

        float tObj = invDet * glm::dot(e2, qvec);
        if (tObj < TRI_EPS || tObj >= tMaxObj) continue;

        float tWorld = tObj * s;
        if (tWorld >= hit.t) continue;

        if(tWorld < closest){
            glm::vec3 pObj = r.At(tObj);
            glm::vec3 pW = glm::vec3(transform * glm::vec4(pObj, 1.0f));

            // Compute world-space normal
            const glm::uvec4& triIdx = (*mesh->triangles)[idx];
            glm::vec3 n0 = (*mesh->normals)[triIdx.x];
            glm::vec3 n1 = (*mesh->normals)[triIdx.y];
            glm::vec3 n2 = (*mesh->normals)[triIdx.z];
            glm::vec3 nObj = glm::normalize((1.0f - u - v) * n0 + u * n1 + v * n2);
            glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(transform)));
            glm::vec3 nW = glm::normalize(normalMatrix * nObj);

            // Interpolate UV coordinates
            if(mesh->uvs){
                hit.uv = (1.0f - u - v) * (*mesh->uvs)[triIdx.x] + 
                        u * (*mesh->uvs)[triIdx.y] + 
                        v * (*mesh->uvs)[triIdx.z];
            } else hit.uv = glm::vec2(0.0f);

            hit.t = tWorld;
            hit.p = pW;
            hit.n = nW;
            hit.front = glm::dot(glm::vec3(transform * glm::vec4(r.d, 0.0f)), nW) < 0.0f;
            hit.materialId = materialId;
            hit.areaLightId = areaLightId;
            hit.shape = this;
            hit.material = material;
            hit.areaLight = areaLight;
            hit.submeshId = i;
			hit.materialId = mesh->materialIndex;
            hitAny = true;
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

TriangleMesh::TriangleMesh(minipbrt::PLYMesh* plyMesh, Scene& scene, uint32_t meshIdx) {
    if (!plyMesh) return;
    // Load mesh via Assimp (with its materials)
    std::string meshPath = plyMesh->filename;
    size_t pos = meshPath.find("./resources/meshes/");
    if (pos != std::string::npos) {
        // Minipbrt works from the scenes dir, so we need to clean the path
        meshPath = meshPath.substr(pos);
    }
    if (!LoadMeshWithAssimp(meshPath, scene, meshIdx)) {
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
}

TriangleMesh::~TriangleMesh() {
    for (auto mesh : meshes) {
        delete mesh;
    }
}

// === Texture loading with Assimp ===
Texture* TriangleMesh::LoadTextureWithAssimp(aiMaterial* aiMat, aiTextureType type, const char* meshName){
        aiString texPath;
        aiReturn texReturn;
        texReturn = aiMat->GetTexture(type, 0, &texPath);
        if(texReturn != aiReturn_SUCCESS){
            // TODO: Proper logging system (with pretty colors!)
            return nullptr;
        }
        Texture* texture = new Texture();
        if(texture->Load(texPath.C_Str())){
            return texture;
        } else {
            return nullptr;
        }
}

// === Mesh loading with Assimp ===
bool TriangleMesh::LoadMeshWithAssimp(const std::string& filename, Scene& scene, uint32_t shapeIdx){
    Assimp::Importer importer;

    // NOTE: Assuming flipped winding order for PBRT
    const aiScene* aiScene = importer.ReadFile(filename,
        aiProcess_Triangulate | 
        aiProcess_GenNormals | 
        aiProcess_FlipWindingOrder);
    
    unsigned int numMeshes = aiScene->mNumMeshes;
    if (!aiScene || numMeshes == 0) {
        std::cerr << "Failed to load mesh: " << filename << std::endl;
        std::cerr << "Error: " << importer.GetErrorString() << std::endl;
        return false;
    }
    
    // Load submeshes and assign their materials 
    for(int i = 0; i < numMeshes; i++){
        const aiMesh* aiMesh = aiScene->mMeshes[i];
        SubMesh* mesh = new SubMesh();
        
        // Load vertices
        mesh->nVerts = aiMesh->mNumVertices;
        mesh->vertices = new std::vector<glm::vec4>(mesh->nVerts);
        for (uint32_t i = 0; i < mesh->nVerts; i++) {
            (*mesh->vertices)[i] = glm::vec4(aiMesh->mVertices[i].x,
                                    aiMesh->mVertices[i].y,
                                    aiMesh->mVertices[i].z,
                                    0.0f);
        }

        // Load triangle indices
        mesh->nTris = aiMesh->mNumFaces;
        mesh->triangles = new std::vector<glm::uvec4>(mesh->nTris);
        for (uint32_t i = 0; i < mesh->nTris; i++) {
            const aiFace& face = aiMesh->mFaces[i];
            (*mesh->triangles)[i] = glm::uvec4(face.mIndices[0], face.mIndices[1], face.mIndices[2], 0u);
        }
        
        // Load normals
        if (aiMesh->HasNormals()) {
            mesh->normals = new std::vector<glm::vec3>(mesh->nVerts);
            for (size_t i = 0; i < mesh->nVerts; i++) {
                (*mesh->normals)[i] = glm::vec3(aiMesh->mNormals[i].x,
                                        aiMesh->mNormals[i].y,
                                        aiMesh->mNormals[i].z);
            }
        } else {
            std::cerr << "  Warning: No normals found for mesh ..." << std::endl;
        }

        // Load UV's
        if(aiMesh->HasTextureCoords(0)){
            mesh->uvs = new std::vector<glm::vec2>(mesh->nVerts);
            for(size_t i = 0; i < mesh->nVerts; i++){
                (*mesh->uvs)[i] = glm::vec2(aiMesh->mTextureCoords[0][i].x,
                                    aiMesh->mTextureCoords[0][i].y);
            }
        } else {
            std::cerr << "  Warning: No UVs found for mesh ..." << std::endl;
        }

        // TODO: For now, assume first submesh uses first PBRT material, etc.
        uint32_t matIdx = shapeIdx + i;
        aiMaterial* aiMat = aiScene->mMaterials[aiMesh->mMaterialIndex];
        if (matIdx < (int)scene.materials.size()) {
            auto disneyMtl = static_cast<DisneyMaterial*>(scene.materials[matIdx]);
            disneyMtl->albedoTexture = LoadTextureWithAssimp(aiMat, aiTextureType_DIFFUSE, aiMesh->mName.C_Str());
            disneyMtl->roughnessTexture = LoadTextureWithAssimp(aiMat, aiTextureType_DIFFUSE_ROUGHNESS, aiMesh->mName.C_Str());
            disneyMtl->metallicTexture = LoadTextureWithAssimp(aiMat, aiTextureType_METALNESS, aiMesh->mName.C_Str());
            mesh->materialIndex = matIdx;
        } else {
			std::cout << "WARNING: Material index out of bounds for submesh in mesh: " << filename << std::endl;
        }

        if(mesh->BuildBVH()){
            mesh->bvhReady = true;
        } else {
            std::cout << "ERROR: Could not build BVH for mesh: " << filename << std::endl;
            return false;
        }

        meshes.push_back(mesh);
    }
    return true;
}

// === BVH Construction ===
bool SubMesh::BuildBVH()
{
    if (!vertices || !triangles) return false;
    if (nVerts == 0 || nTris == 0) return false;
    if (vertices->size() < nVerts) return false;
    if (triangles->size() < nTris) return false;

    bvhTriSoup.resize(size_t(nTris) * 3ull);

    for (uint32_t i = 0; i < nTris; i++)
    {
        const glm::uvec4 t = (*triangles)[i];
        if (t.x >= nVerts || t.y >= nVerts || t.z >= nVerts) return false;

        const size_t base = size_t(i) * 3ull;
        bvhTriSoup[base + 0] = (*vertices)[t.x];
        bvhTriSoup[base + 1] = (*vertices)[t.y];
        bvhTriSoup[base + 2] = (*vertices)[t.z];
    }

    bvh.BuildHQ(reinterpret_cast<const tinybvh::bvhvec4*>(bvhTriSoup.data()), nTris);
    return true;
}

