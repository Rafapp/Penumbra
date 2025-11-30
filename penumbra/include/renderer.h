#pragma once

#include <iostream>
#include <thread>

#include "minipbrt.h"

#include "scene.h"
#include "threading.h"
#include "pbrtconverter.h"

#define PATHTRACING_SPP 0 
#define MIN_SPP 1 
#define MAX_SPP 8 
#define BOUNCES 3 
#define PHOTON_MAX_BOUNCES 1000 
#define SKIP_PHOTONMAP_DIRECT true
#define SHADING_ERROR_THRESHOLD 1e-1f
#define LIGHTING_ERROR_THRESHOLD 1e-8f
inline unsigned int NTHREADS = (int)std::thread::hardware_concurrency();
// inline unsigned int NTHREADS = 2;

class Renderer{
public:
    Renderer();
    ~Renderer();
    bool SetPbrtScene(minipbrt::Scene* scene);
    void RenderPixel(int u, int v);
    bool IntersectRayScene(const Ray& ray, HitInfo& hitInfo);
    void BeginRender();
    void StopRender();
    bool LoadScene(const std::string& filename);
    int GetRenderWidth() const { return renderWidth; }
    int GetRenderHeight() const { return renderHeight; }
    std::vector<uint8_t>& GetRenderBuffer() { return renderBuffer; }

private:
    std::string sceneFilename;
    std::vector<uint8_t> renderBuffer;
    std::unique_ptr<Scene> scene;
    minipbrt::Scene* pbrtScene = nullptr;
    std::unique_ptr<RenderThreadPool> threadPool;

    // TODO: Grab from GUI
    int renderWidth = 960;
    int renderHeight = 540;
    void ConvertPbrtScene();
};