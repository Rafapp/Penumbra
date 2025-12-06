#pragma once

#include <iostream>
#include <thread>

#include "minipbrt.h"

#include "scene.h"
#include "threading.h"
#include "pbrtconverter.h"
#include "sampling.h"
#include "gui.h"

// TODO: Adaptive SPP
// #define MIN_SPP 1 
// #define MAX_SPP 8 
// #define SHADING_ERROR_THRESHOLD 1e-1f
// #define LIGHTING_ERROR_THRESHOLD 1e-1f

#define SHADOW_EPS 1e-4f
#define SPP 1 
#define MAX_BOUNCES 0 
#define INF_BOUNCES true
inline unsigned int NTHREADS = (int)std::thread::hardware_concurrency();
// inline unsigned int NTHREADS = 2;

class Renderer{
public:
    Renderer();
    ~Renderer();
    bool SetPbrtScene(minipbrt::Scene* scene);
    void RenderPixel(int u, int v);
    bool IntersectRayScene(const Ray& ray, HitInfo& hit) const;
    float TraceShadowRay(const glm::vec3& o, const glm::vec3& d, const glm::vec3& n, float maxDist) const ;
    glm::vec3 TracePath(const Ray& ray, Sampler& sampler, int depth);
    void BeginRender();
    void StopRender();
    bool LoadScene(const std::string& filename);
    int GetRenderWidth() const { return renderWidth; }
    int GetRenderHeight() const { return renderHeight; }
    void SetRenderWidth(int w) { renderWidth = w; }
    void SetRenderHeight(int h) { renderHeight = h; }
    std::vector<uint8_t>& GetRenderBuffer() { return renderBuffer; }
    void SetGUI(GUI* guiPtr) { gui = guiPtr; }

private:
    std::string sceneFilename;
    std::vector<uint8_t> renderBuffer;
    std::unique_ptr<Scene> scene;
    minipbrt::Scene* pbrtScene = nullptr;
    std::unique_ptr<RenderThreadPool> threadPool;


    // GUI Variables
    GUI* gui = nullptr;
    int renderWidth = -1;
    int renderHeight = -1;

    void ConvertPbrtScene();
};