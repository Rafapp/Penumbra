#include "pbrtconverter.h"
#include "scene.h"
#include "camera.h"
#include "shapes.h"

Scene PbrtConverter::ConvertScene(minipbrt::Scene* pbrtScene) {
    Scene scene;

    for (auto pbrtShape : pbrtScene->shapes) {
        Shape* shape = ConvertShape(pbrtShape);
        if (shape) scene.shapes.push_back(shape);
    }
    
    scene.camera = ConvertCamera(pbrtScene->camera);
    
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

glm::mat4 PbrtConverter::TransformToMat4(const minipbrt::Transform& transform) {
    glm::mat4 mat(
        glm::vec4(transform.start[0][0], transform.start[0][1], transform.start[0][2], transform.start[0][3]),
        glm::vec4(transform.start[1][0], transform.start[1][1], transform.start[1][2], transform.start[1][3]),
        glm::vec4(transform.start[2][0], transform.start[2][1], transform.start[2][2], transform.start[2][3]),
        glm::vec4(transform.start[3][0], transform.start[3][1], transform.start[3][2], transform.start[3][3])
    );
    return glm::transpose(mat);
}