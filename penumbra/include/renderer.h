#pragma once

#include <iostream>

#include "minipbrt.h"

#include "scene.h"
#include "pbrtconverter.h"

class Renderer{
public:
    Renderer();
    ~Renderer();
    bool SetPbrtScene(minipbrt::Scene* scene);
private:
    std::unique_ptr<Scene> scene;
    minipbrt::Scene* pbrtScene = nullptr;
    void ConvertPbrtScene();
};