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

Scene PbrtConverter::ConvertScene(minipbrt::Scene* pbrtScene) {
    Scene scene;

    // Shapes
    for (auto pbrtShape : pbrtScene->shapes) {
        Shape* shape = ConvertShape(pbrtShape);
        if (shape) scene.shapes.push_back(shape);
    }

    // Lights
    for( auto pbrtLight : pbrtScene->lights) {
        Light* light = ConvertLight(pbrtLight);
        if (light) scene.lights.push_back(light);
    }
    
    // Materials
    for (auto pbrtMat : pbrtScene->materials) {
        Material* material = ConvertMaterial(pbrtMat);
        if (material) scene.materials.push_back(material);
    }

    // Camera
    scene.camera = ConvertCamera(pbrtScene->camera);
    return scene;
}

Light* PbrtConverter::ConvertLight(minipbrt::Light* pbrtLight) {
    Light* light = nullptr;

    // Check light type and convert accordingly
    if (pbrtLight->type() == minipbrt::LightType::Point) {
        auto pbrtPointLight = static_cast<minipbrt::PointLight*>(pbrtLight);
        light = new PointLight(pbrtPointLight);
    } 
    return light;
}

Material* PbrtConverter::ConvertMaterial(minipbrt::Material* pbrtMat) {
    if (!pbrtMat) return nullptr;
    
    Material* material = nullptr;
    
    if (pbrtMat->type() == minipbrt::MaterialType::Matte) {
        material = new MatteMaterial(static_cast<minipbrt::MatteMaterial*>(pbrtMat));
    }
    
    return material;
}

Shape* PbrtConverter::ConvertShape(minipbrt::Shape* pbrtShape) {
    Shape* shape = nullptr;
    
    if (pbrtShape->type() == minipbrt::ShapeType::Sphere) {
        auto pbrtSphere = static_cast<minipbrt::Sphere*>(pbrtShape);
        shape = new Sphere(pbrtSphere);
    } 
    else if (pbrtShape->type() == minipbrt::ShapeType::PLYMesh) {
        auto plyMesh = static_cast<minipbrt::PLYMesh*>(pbrtShape);
        shape = new TriangleMesh(plyMesh);
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