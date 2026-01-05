#include "pbrtconverter.h"

#include "scene.h"
#include "camera.h"
#include "shapes.h"
#include "lights.h"
#include "materials.h"
#include <glm/gtc/type_ptr.hpp>

glm::mat4 PbrtConverter::TransformToMat4(const minipbrt::Transform& t) {
    glm::mat4 m;
    for (int r = 0; r < 4; r++)
        for (int c = 0; c < 4; c++)
            m[c][r] = t.start[r][c];
    return m;
}

// TODO: Instancing to reduce duplicate data in meshes
// === Scene conversion ===
Scene PbrtConverter::ConvertScene(minipbrt::Scene* pbrtScene) {
    Scene scene;

    // 1. PBRT materials 
    for (auto pbrtMat : pbrtScene->materials) {
        Material* material = ConvertMaterial(pbrtMat);
		scene.materials.push_back(material); 
    }

    // 2. Area Lights
    for (auto pbrtAreaLight : pbrtScene->areaLights) {
        AreaLight* areaLight = ConvertAreaLight(pbrtAreaLight);
        if (areaLight) {
            scene.lights.push_back(areaLight);
        }
    }

    // 3. Shapes
    for (auto pbrtShape : pbrtScene->shapes) {
        Shape* shape = ConvertShape(pbrtShape, scene);
        if (shape) {
            scene.shapes.push_back(shape);
            uint32_t lightIdx = pbrtShape->areaLight;
            if (lightIdx != minipbrt::kInvalidIndex) {
                AreaLight* areaLight = static_cast<AreaLight*>(scene.lights[lightIdx]);
                if (areaLight) {
                    areaLight->shape = shape;
                    shape->areaLight = areaLight;
                } else {
                    throw std::runtime_error("Area light index " + std::to_string(lightIdx) + " not found for shape.");
                }
            }
		}
    }

    // 4. Assign materials to shapes
	// TODO: Pass index to LoadWithAssimp to assign materials directly 
    int shapeIdx = 0;
    for (auto pbrtShape : pbrtScene->shapes) {
        if (shapeIdx >= scene.shapes.size()) break;

        // Skip mesh shapes - they handle materials in LoadMeshWithAssimp
        if (auto mesh = dynamic_cast<TriangleMesh*>(scene.shapes[shapeIdx])) {
            shapeIdx++;
            continue;
        }

        // Assign PBRT material to non-mesh shapes
        int mi = pbrtShape->material;
        if (mi != minipbrt::kInvalidIndex && mi < (int)scene.materials.size()) {
            Material* m = scene.materials[mi];
            if (m) scene.shapes[shapeIdx]->material = m;
        }
        shapeIdx++;
    }

    // Ideal lights
    for( auto pbrtIdealLight : pbrtScene->lights) {
        IdealLight* idealLight = ConvertIdealLight(pbrtIdealLight);
        if (idealLight) {
            scene.lights.push_back(idealLight);
        }
    }

    // Camera
    scene.camera = ConvertCamera(pbrtScene->camera);
    return scene;
}

// === Primitive conversion ===
IdealLight* PbrtConverter::ConvertIdealLight(minipbrt::Light* pbrtLight) {
    IdealLight* light = nullptr;

    // Check light type and convert accordingly
    if (pbrtLight->type() == minipbrt::LightType::Point) {
        auto pbrtPointLight = static_cast<minipbrt::PointLight*>(pbrtLight);
        light = new PointLight(pbrtPointLight);
    } 
    return light;
}

AreaLight* PbrtConverter::ConvertAreaLight(minipbrt::AreaLight* pbrtAreaLight) {
    AreaLight* areaLight = nullptr;

    // Check area light type and convert accordingly
    if(pbrtAreaLight->type() == minipbrt::AreaLightType::Diffuse) {
        auto pbrtDiffuseAreaLight = static_cast<minipbrt::DiffuseAreaLight*>(pbrtAreaLight);
        areaLight = new DiffuseAreaLight(pbrtDiffuseAreaLight);
    }
    return areaLight;
}

Material* PbrtConverter::ConvertMaterial(minipbrt::Material* pbrtMat) {
    if (!pbrtMat) return nullptr;
    
    Material* material = nullptr;
    
    if (pbrtMat->type() == minipbrt::MaterialType::Matte) {
        material = new MatteMaterial(static_cast<minipbrt::MatteMaterial*>(pbrtMat));
    } else if (pbrtMat->type() == minipbrt::MaterialType::Disney) {
        material = new DisneyMaterial(static_cast<minipbrt::DisneyMaterial*>(pbrtMat));
    } else {
        std::cerr << "Unsupported material type" << std::endl;
    }

    return material;
}

Shape* PbrtConverter::ConvertShape(minipbrt::Shape* pbrtShape, Scene& scene) {
    Shape* shape = nullptr;
    
    if (pbrtShape->type() == minipbrt::ShapeType::Sphere) {
        auto pbrtSphere = static_cast<minipbrt::Sphere*>(pbrtShape);
        shape = new Sphere(pbrtSphere);
    } 
    else if (pbrtShape->type() == minipbrt::ShapeType::PLYMesh) {
        auto plyMesh = static_cast<minipbrt::PLYMesh*>(pbrtShape);
        shape = new TriangleMesh(plyMesh, scene);
    }
    else {
        std::cerr << "Unsupported shape type" << std::endl;
    }
    
    return shape;
}

Camera* PbrtConverter::ConvertCamera(minipbrt::Camera* pbrtCam) {
    if (!pbrtCam) return nullptr;
    
    Camera* camera = nullptr;
    
    // Check camera type and convert accordingly
    if (pbrtCam->type() == minipbrt::CameraType::Perspective) {
        auto pbrtPerspCam = static_cast<minipbrt::PerspectiveCamera*>(pbrtCam);
        camera = new PerspectiveCamera(pbrtPerspCam);
    } 
    // TODO: Add other camera types
    
    return camera;
}