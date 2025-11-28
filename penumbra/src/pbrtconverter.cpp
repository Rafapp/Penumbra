#include "pbrtconverter.h"
#include "shapes.h"
#include "materials.h"

Scene PbrtConverter::ConvertScene(minipbrt::Scene* pbrtScene) {
    Scene scene;
    std::cout << "pbrt scene has " << pbrtScene->shapes.size() << " shapes." << std::endl;
    
    for (auto pbrtShape : pbrtScene->shapes) {
        std::cout << "Converting PBRT Shape to Penumbra Shape..." << std::endl;
        Shape* shape = ConvertShape(pbrtShape);
        if (shape) scene.shapes.push_back(shape);
    }
    
    // scene.camera = ConvertCamera(pbrtScene->camera);
    
    return scene;
}

Shape* PbrtConverter::ConvertShape(minipbrt::Shape* pbrtShape) {
    Shape* shape = nullptr;
    
    // Check shape type and convert accordingly
    if (pbrtShape->type() == minipbrt::ShapeType::Sphere) {
        auto pbrtSphere = static_cast<minipbrt::Sphere*>(pbrtShape);
        shape = new Sphere(pbrtSphere);
    } 
    return shape;
}